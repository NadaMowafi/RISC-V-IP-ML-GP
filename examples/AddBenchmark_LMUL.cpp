#include "ImageReader.hpp"
#include "ImageWriter.hpp"
#include "AddRVV.hpp"
#include "Add.hpp"
#include "VectorTraits_LMUL.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

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

template<int LMUL>
std::vector<std::vector<uint8_t>> add_LMUL_impl(const std::vector<std::vector<uint8_t>>& img1, 
                                                 const std::vector<std::vector<uint8_t>>& img2) {
    const int height = img1.size();
    const int width = img1[0].size();
    std::vector<std::vector<uint8_t>> result(height, std::vector<uint8_t>(width));
    
    using Traits = VectorTraits_LMUL<uint8_t, LMUL>;
    
    for (int i = 0; i < height; ++i) {
        int j = 0;
        while (j < width) {
            size_t vl = Traits::vsetvl(width - j);
            auto va = Traits::vle(&img1[i][j], vl);
            auto vb = Traits::vle(&img2[i][j], vl);
            auto vsum = Traits::vsaddu_vv(va, vb, vl);  // Saturating add to prevent overflow
            Traits::vse(&result[i][j], vsum, vl);
            j += vl;
        }
    }
    return result;
}

int main() {
    ImageReader reader;
    ImageWriter writer;
    Image image;

    const int iterations = 100;

    // Read input image once
    ImageStatus status = reader.readImage("barb.512.pgm", image);
    if (status != ImageStatus::SUCCESS) {
        std::cerr << "Failed to read image: " << static_cast<int>(status) << std::endl;
        return 1;
    }

    std::cout << "=== Image Addition LMUL Comparison ===" << std::endl;
    std::cout << "Image size: " << image.pixelMatrix.size() << " x " << image.pixelMatrix[0].size() << std::endl;
    std::cout << "Iterations: " << iterations << std::endl << std::endl;

    // Benchmark scalar reference
    double time_scalar;
    {
        auto scalar_fn = [&]() {
            auto result = add(image.pixelMatrix, image.pixelMatrix, CLIPPING::SATURATION);
        };
        time_scalar = bench_ms(scalar_fn, iterations);
    }

    // Benchmark original vector implementation
    double time_original;
    {
        auto vector_fn = [&]() {
            auto result = __riscv_Add(image.pixelMatrix, image.pixelMatrix, CLIPPING_V::SATURATION);
        };
        time_original = bench_ms(vector_fn, iterations);
    }

    // Benchmark LMUL variants
    std::vector<std::pair<std::string, double>> lmul_results;
    
    // LMUL=1
    {
        auto lmul1_fn = [&]() {
            auto result = add_LMUL_impl<1>(image.pixelMatrix, image.pixelMatrix);
        };
        double time_m1 = bench_ms(lmul1_fn, iterations);
        lmul_results.push_back({"m1", time_m1});
    }
    
    // LMUL=2
    {
        auto lmul2_fn = [&]() {
            auto result = add_LMUL_impl<2>(image.pixelMatrix, image.pixelMatrix);
        };
        double time_m2 = bench_ms(lmul2_fn, iterations);
        lmul_results.push_back({"m2", time_m2});
    }
    
    // LMUL=4
    {
        auto lmul4_fn = [&]() {
            auto result = add_LMUL_impl<4>(image.pixelMatrix, image.pixelMatrix);
        };
        double time_m4 = bench_ms(lmul4_fn, iterations);
        lmul_results.push_back({"m4", time_m4});
    }
    
    // LMUL=8
    {
        auto lmul8_fn = [&]() {
            auto result = add_LMUL_impl<8>(image.pixelMatrix, image.pixelMatrix);
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
    std::cout << "Best LMUL for image addition: ";
    
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

    return 0;
} 