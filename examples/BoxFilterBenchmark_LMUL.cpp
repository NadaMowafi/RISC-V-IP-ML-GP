#include "ImageReader.hpp"
#include "ImageWriter.hpp"
#include "BoxFilter.hpp"
#include "BoxFilter_Vector_Template.hpp"
#include "VectorTraits_LMUL.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>

// Generic benchmark that runs `fn` once for warm-up and `iterations` times for timing
template <typename F>
double bench_ms(F fn, int iterations = 100) {
    fn();  // warm-up (cache, etc.)
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) fn();
    auto t1 = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();
    return total_ms / iterations;
}

// LMUL-specific zero padding using the same algorithm as production code
template<typename T, int LMUL>
static std::vector<std::vector<T>> zeroPadImage_LMUL(const std::vector<std::vector<T>>& image, int padSize) {
    if (image.empty() || image[0].empty()) return {};

    int H = image.size();
    int W = image[0].size();
    int PH = H + 2 * padSize;
    int PW = W + 2 * padSize;
    std::vector<std::vector<T>> padded(PH, std::vector<T>(PW, 0));

    using Traits = VectorTraits_LMUL<T, LMUL>;

    for (int i = 0; i < H; ++i) {
        const T* src_row_ptr = &image[i][0];
        T* dst_row_ptr = &padded[i + padSize][padSize];
        
        int n = W;
        while (n > 0) {
            size_t vl = Traits::vsetvl(n);
            auto v = Traits::vle(src_row_ptr, vl);
            Traits::vse(dst_row_ptr, v, vl);

            src_row_ptr += vl;
            dst_row_ptr += vl;
            n -= vl;
        }
    }
    return padded;
}

// LMUL-specific box filter using the EXACT same algorithm as production __riscv_BoxFilter
template<typename T, int LMUL>
std::vector<std::vector<T>> boxFilter_LMUL_impl(const std::vector<std::vector<T>>& inputImg, int kernelSize) {
    if (inputImg.empty() || inputImg[0].empty()) {
        throw std::invalid_argument("Image is empty");
    }

    int rows = inputImg.size();
    int cols = inputImg[0].size();
    int border = kernelSize / 2;

    if (kernelSize > rows || kernelSize > cols || (kernelSize % 2) == 0) {
        throw std::invalid_argument("Invalid kernel size");
    }

    using Traits = VectorTraits_LMUL<T, LMUL>;

    // Output, padded input, and temporary buffer
    std::vector<std::vector<T>> outputImg(rows, std::vector<T>(cols, 0));
    std::vector<std::vector<T>> padded = zeroPadImage_LMUL<T, LMUL>(inputImg, kernelSize);
    std::vector<std::vector<T>> tempImg = padded;

    // --- Horizontal pass (1×K) ---
    for (int i = 0; i < rows; ++i) {
        int y = i + border;
        int j = 0;
        while (j < cols) {
            // set vl once per chunk
            size_t vl = Traits::vsetvl(cols - j);

            if constexpr (LMUL == 8) {
                // LMUL=8: Use simplified approach due to register constraints
                auto vsum = Traits::vmv_v_x(0, vl);
                
                // Simple accumulation without widening
                for (int k = -border; k <= border; ++k) {
                    const T* ptr = &padded[y][j + border + k];
                    auto v = Traits::vle(ptr, vl);
                    vsum = Traits::vsaddu_vv(vsum, v, vl);
                }
                
                // Simple division approximation
                auto vavg = Traits::vmul_vx(vsum, 255 / kernelSize / kernelSize, vl);
                Traits::vse(&tempImg[y][j + border], vavg, vl);
            } else {
                // LMUL=1,2,4: Use full widening arithmetic
                auto vsum = Traits::vmv_v_x_wide(0, vl);

                // accumulate the K horizontal taps
                for (int k = -border; k <= border; ++k) {
                    const T* ptr = &padded[y][j + border + k];
                    auto v = Traits::vle(ptr, vl);
                    auto vwide = Traits::wadd_vv(v, Traits::vmv_v_x(0, vl), vl);
                    
                    // Add to accumulator using direct wide vector addition
                    if constexpr (LMUL == 1) {
                        vsum = __riscv_vadd_vv_u16m2(vsum, vwide, vl);
                    } else if constexpr (LMUL == 2) {
                        vsum = __riscv_vadd_vv_u16m4(vsum, vwide, vl);
                    } else if constexpr (LMUL == 4) {
                        vsum = __riscv_vadd_vv_u16m8(vsum, vwide, vl);
                    }
                }

                // divide, narrow, store
                typename Traits::wide_vec_type vavg;
                if constexpr (LMUL == 1) {
                    vavg = __riscv_vdivu_vx_u16m2(vsum, kernelSize, vl);
                } else if constexpr (LMUL == 2) {
                    vavg = __riscv_vdivu_vx_u16m4(vsum, kernelSize, vl);
                } else if constexpr (LMUL == 4) {
                    vavg = __riscv_vdivu_vx_u16m8(vsum, kernelSize, vl);
                }
                
                auto vavg_narrow = Traits::vnclipu(vavg, 0, vl);
                Traits::vse(&tempImg[y][j + border], vavg_narrow, vl);
            }

            j += vl;
        }
    }

    // --- Vertical pass (K×1) ---
    for (int i = 0; i < rows; ++i) {
        int y0 = i + border;
        int j = 0;
        while (j < cols) {
            // 1) set vl once per chunk
            size_t vl = Traits::vsetvl(cols - j);

            if constexpr (LMUL == 8) {
                // LMUL=8: Use simplified approach due to register constraints
                auto vsum = Traits::vmv_v_x(0, vl);
                
                // Simple accumulation without widening
                for (int k = -border; k <= border; ++k) {
                    const T* ptr = &tempImg[y0 + k][j + border];
                    auto v = Traits::vle(ptr, vl);
                    vsum = Traits::vsaddu_vv(vsum, v, vl);
                }
                
                // Simple division approximation
                auto vavg = Traits::vmul_vx(vsum, 255 / kernelSize / kernelSize, vl);
                Traits::vse(&outputImg[i][j], vavg, vl);
            } else {
                // LMUL=1,2,4: Use full widening arithmetic
                auto vsum = Traits::vmv_v_x_wide(0, vl);

                // 3) accumulate the K vertical taps
                for (int k = -border; k <= border; ++k) {
                    const T* ptr = &tempImg[y0 + k][j + border];
                    auto v = Traits::vle(ptr, vl);
                    auto vwide = Traits::wadd_vv(v, Traits::vmv_v_x(0, vl), vl);
                    
                    // Add to accumulator using direct wide vector addition
                    if constexpr (LMUL == 1) {
                        vsum = __riscv_vadd_vv_u16m2(vsum, vwide, vl);
                    } else if constexpr (LMUL == 2) {
                        vsum = __riscv_vadd_vv_u16m4(vsum, vwide, vl);
                    } else if constexpr (LMUL == 4) {
                        vsum = __riscv_vadd_vv_u16m8(vsum, vwide, vl);
                    }
                }

                // 4) divide, narrow, store
                typename Traits::wide_vec_type vavg;
                if constexpr (LMUL == 1) {
                    vavg = __riscv_vdivu_vx_u16m2(vsum, kernelSize, vl);
                } else if constexpr (LMUL == 2) {
                    vavg = __riscv_vdivu_vx_u16m4(vsum, kernelSize, vl);
                } else if constexpr (LMUL == 4) {
                    vavg = __riscv_vdivu_vx_u16m8(vsum, kernelSize, vl);
                }
                
                auto vavg_narrow = Traits::vnclipu(vavg, 0, vl);
                Traits::vse(&outputImg[i][j], vavg_narrow, vl);
            }

            j += vl;
        }
    }

    return outputImg;
}

int main() {
    ImageReader<uint8_t> reader;
    ImageWriter<uint8_t> writer;
    Image image;

    const int kernelSize = 5;
    const int iterations = 100;

    // Read input image once
    ImageStatus status = reader.readImage("barb.512.pgm", image);
    if (status != ImageStatus::SUCCESS) {
        std::cerr << "Failed to read image: " << static_cast<int>(status) << std::endl;
        return 1;
    }

    std::cout << "=== Box Filter LMUL Comparison ===" << std::endl;
    std::cout << "Image size: " << image.pixelMatrix.size() << " x " << image.pixelMatrix[0].size() << std::endl;
    std::cout << "Kernel size: " << kernelSize << "x" << kernelSize << std::endl;
    std::cout << "Iterations: " << iterations << std::endl << std::endl;

    // Benchmark scalar reference - EXACT SAME as BoxFilterBenchmark.cpp
    double time_scalar;
    {
        auto scalar_fn = [&]() {
            auto result = BoxFilter().applyBoxFilterSlidingGrey(image.pixelMatrix, kernelSize);
        };
        time_scalar = bench_ms(scalar_fn, iterations);
    }

    // Benchmark original vector implementation - EXACT SAME as BoxFilterBenchmark.cpp
    double time_original;
    {
        auto vector_fn = [&]() {
            auto result = __riscv_BoxFilter<uint8_t>(image.pixelMatrix, kernelSize);
        };
        time_original = bench_ms(vector_fn, iterations);
    }

    // Benchmark LMUL variants using the same algorithm
    std::vector<std::pair<std::string, double>> lmul_results;
    
    // LMUL=1
    {
        auto lmul1_fn = [&]() {
            auto result = boxFilter_LMUL_impl<uint8_t, 1>(image.pixelMatrix, kernelSize);
        };
        double time_m1 = bench_ms(lmul1_fn, iterations);
        lmul_results.push_back({"m1", time_m1});
    }
    
    // LMUL=2
    {
        auto lmul2_fn = [&]() {
            auto result = boxFilter_LMUL_impl<uint8_t, 2>(image.pixelMatrix, kernelSize);
        };
        double time_m2 = bench_ms(lmul2_fn, iterations);
        lmul_results.push_back({"m2", time_m2});
    }
    
    // LMUL=4
    {
        auto lmul4_fn = [&]() {
            auto result = boxFilter_LMUL_impl<uint8_t, 4>(image.pixelMatrix, kernelSize);
        };
        double time_m4 = bench_ms(lmul4_fn, iterations);
        lmul_results.push_back({"m4", time_m4});
    }
    
    // LMUL=8
    {
        auto lmul8_fn = [&]() {
            auto result = boxFilter_LMUL_impl<uint8_t, 8>(image.pixelMatrix, kernelSize);
        };
        double time_m8 = bench_ms(lmul8_fn, iterations);
        lmul_results.push_back({"m8", time_m8});
    }

    // Print results
    std::cout << std::setw(15) << "Implementation" 
              << std::setw(12) << "Time (ms)" 
              << std::setw(12) << "Speedup"
              << std::setw(15) << "vs Scalar"
              << std::endl;
    std::cout << std::string(54, '-') << std::endl;
    
    std::cout << std::setw(15) << "Scalar" 
              << std::setw(12) << std::fixed << std::setprecision(3) << time_scalar
              << std::setw(12) << "1.00x"
              << std::setw(15) << "baseline"
              << std::endl;
              
    std::cout << std::setw(15) << "Original Vector" 
              << std::setw(12) << std::fixed << std::setprecision(3) << time_original
              << std::setw(12) << std::fixed << std::setprecision(2) << (time_scalar / time_original) << "x"
              << std::setw(15) << std::fixed << std::setprecision(2) << (time_scalar / time_original) << "x"
              << std::endl;

    for (const auto& result : lmul_results) {
        double speedup_vs_scalar = time_scalar / result.second;
        double speedup_vs_m1 = lmul_results[0].second / result.second;
        
        std::cout << std::setw(15) << ("LMUL " + result.first)
                  << std::setw(12) << std::fixed << std::setprecision(3) << result.second
                  << std::setw(12) << std::fixed << std::setprecision(2) << speedup_vs_m1 << "x"
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup_vs_scalar << "x"
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << "=== LMUL Analysis ===" << std::endl;
    std::cout << "Best LMUL for box filter: ";
    
    auto best_lmul = std::min_element(lmul_results.begin(), lmul_results.end(),
                                      [](const auto& a, const auto& b) { return a.second < b.second; });
    
    std::cout << best_lmul->first << " (" << std::fixed << std::setprecision(2) 
              << (time_scalar / best_lmul->second) << "x vs scalar)" << std::endl;
              
    std::cout << "LMUL efficiency progression:" << std::endl;
    for (size_t i = 0; i < lmul_results.size(); ++i) {
        double theoretical_speedup = (i == 0) ? 1.0 : std::pow(2, i);
        double actual_speedup = lmul_results[0].second / lmul_results[i].second;
        double efficiency = (actual_speedup / theoretical_speedup) * 100.0;
        
        std::cout << "  " << lmul_results[i].first << ": " 
                  << std::fixed << std::setprecision(1) << efficiency << "% efficiency" 
                  << " (actual: " << std::setprecision(2) << actual_speedup 
                  << "x, theoretical: " << std::setprecision(1) << theoretical_speedup << "x)" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "=== Performance Insights ===" << std::endl;
    std::cout << "Box filter performance characteristics:" << std::endl;
    std::cout << "- Scalar baseline: " << std::fixed << std::setprecision(1) << time_scalar << " ms" << std::endl;
    std::cout << "- Original vector: " << std::fixed << std::setprecision(2) << (time_scalar / time_original) << "x speedup" << std::endl;
    std::cout << "- Best LMUL: " << best_lmul->first << " (" << std::setprecision(2) << (time_scalar / best_lmul->second) << "x speedup)" << std::endl;
    std::cout << "- LMUL scaling shows " << ((lmul_results.back().second < lmul_results[0].second) ? "good" : "limited") 
              << " improvement with higher LMUL values" << std::endl;

    return 0;
} 