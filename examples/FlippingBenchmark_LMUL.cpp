#include "ImageReader.hpp"
#include "ImageWriter.hpp"
#include "Flipping.hpp"
#include "Flipping_Vector.hpp"
#include "VectorTraits_LMUL.hpp"
#include "Image.hpp"
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
std::vector<std::vector<uint8_t>> flip_horizontal_LMUL(const std::vector<std::vector<uint8_t>>& img) {
    const int height = img.size();
    const int width = img[0].size();
    std::vector<std::vector<uint8_t>> result(height, std::vector<uint8_t>(width));
    
    using Traits = VectorTraits_LMUL<uint8_t, LMUL>;
    
    for (int i = 0; i < height; ++i) {
        // Process row in chunks, then reverse each chunk
        int j = 0;
        while (j < width) {
            size_t chunk_size = std::min(static_cast<size_t>(width - j), 
                                       static_cast<size_t>(Traits::vsetvl(width - j)));
            size_t vl = Traits::vsetvl(chunk_size);
            
            auto vec = Traits::vle(&img[i][j], vl);
            auto reversed = Traits::reverse_vector(vec, vl);
            Traits::vse(&result[i][width - j - vl], reversed, vl);
            j += vl;
        }
    }
    return result;
}

template<int LMUL>
std::vector<std::vector<uint8_t>> flip_vertical_LMUL(const std::vector<std::vector<uint8_t>>& img) {
    const int height = img.size();
    const int width = img[0].size();
    std::vector<std::vector<uint8_t>> result(height, std::vector<uint8_t>(width));
    
    using Traits = VectorTraits_LMUL<uint8_t, LMUL>;
    
    for (int i = 0; i < height; ++i) {
        int src_row = height - 1 - i;
        int j = 0;
        while (j < width) {
            size_t vl = Traits::vsetvl(width - j);
            auto vec = Traits::vle(&img[src_row][j], vl);
            Traits::vse(&result[i][j], vec, vl);
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

    std::cout << "=== Image Flipping LMUL Comparison ===" << std::endl;
    std::cout << "Image size: " << image.pixelMatrix.size() << " x " << image.pixelMatrix[0].size() << std::endl;
    std::cout << "Iterations: " << iterations << std::endl << std::endl;

    // Create flipper objects for reference benchmarks
    ImageFlipper<uint8_t> scalarFlipper;
    ImageFlipper_V<uint8_t> vectorFlipper;

    // ===== HORIZONTAL FLIPPING BENCHMARKS =====
    std::cout << "=== HORIZONTAL FLIPPING ===" << std::endl;
    
    // Benchmark scalar reference
    double time_scalar_h;
    {
        auto scalar_fn = [&]() {
            Image copy = image;
            scalarFlipper.flip(copy, FlippingDirection::HORIZONTAL);
        };
        time_scalar_h = bench_ms(scalar_fn, iterations);
    }

    // Benchmark original vector implementation
    double time_original_h;
    {
        auto vector_fn = [&]() {
            Image copy = image;
            vectorFlipper.flip(copy, FlippingDirection_V::HORIZONTAL);
        };
        time_original_h = bench_ms(vector_fn, iterations);
    }

    // Benchmark LMUL variants for horizontal flipping
    std::vector<std::pair<std::string, double>> lmul_results_h;
    
    // LMUL=1
    {
        auto lmul1_fn = [&]() {
            auto result = flip_horizontal_LMUL<1>(image.pixelMatrix);
        };
        double time_m1 = bench_ms(lmul1_fn, iterations);
        lmul_results_h.push_back({"m1", time_m1});
    }
    
    // LMUL=2
    {
        auto lmul2_fn = [&]() {
            auto result = flip_horizontal_LMUL<2>(image.pixelMatrix);
        };
        double time_m2 = bench_ms(lmul2_fn, iterations);
        lmul_results_h.push_back({"m2", time_m2});
    }
    
    // LMUL=4
    {
        auto lmul4_fn = [&]() {
            auto result = flip_horizontal_LMUL<4>(image.pixelMatrix);
        };
        double time_m4 = bench_ms(lmul4_fn, iterations);
        lmul_results_h.push_back({"m4", time_m4});
    }
    
    // LMUL=8
    {
        auto lmul8_fn = [&]() {
            auto result = flip_horizontal_LMUL<8>(image.pixelMatrix);
        };
        double time_m8 = bench_ms(lmul8_fn, iterations);
        lmul_results_h.push_back({"m8", time_m8});
    }

    // Print horizontal flipping results
    std::cout << std::setw(15) << "Implementation" 
              << std::setw(12) << "Time (ms)" 
              << std::setw(12) << "Speedup"
              << std::setw(15) << "vs Scalar"
              << std::endl;
    std::cout << std::string(54, '-') << std::endl;
    
    std::cout << std::setw(15) << "Scalar" 
              << std::setw(12) << std::fixed << std::setprecision(3) << time_scalar_h
              << std::setw(12) << "1.00x"
              << std::setw(15) << "baseline"
              << std::endl;
              
    std::cout << std::setw(15) << "Original Vector" 
              << std::setw(12) << std::fixed << std::setprecision(3) << time_original_h
              << std::setw(12) << std::fixed << std::setprecision(2) << (time_scalar_h / time_original_h) << "x"
              << std::setw(15) << std::fixed << std::setprecision(2) << (time_scalar_h / time_original_h) << "x"
              << std::endl;

    for (const auto& result : lmul_results_h) {
        double speedup_vs_scalar = time_scalar_h / result.second;
        double speedup_vs_m1 = lmul_results_h[0].second / result.second;
        
        std::cout << std::setw(15) << ("LMUL " + result.first)
                  << std::setw(12) << std::fixed << std::setprecision(3) << result.second
                  << std::setw(12) << std::fixed << std::setprecision(2) << speedup_vs_m1 << "x"
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup_vs_scalar << "x"
                  << std::endl;
    }

    // ===== VERTICAL FLIPPING BENCHMARKS =====
    std::cout << std::endl << "=== VERTICAL FLIPPING ===" << std::endl;
    
    // Benchmark scalar reference
    double time_scalar_v;
    {
        auto scalar_fn = [&]() {
            Image copy = image;
            scalarFlipper.flip(copy, FlippingDirection::VERTICAL);
        };
        time_scalar_v = bench_ms(scalar_fn, iterations);
    }

    // Benchmark original vector implementation
    double time_original_v;
    {
        auto vector_fn = [&]() {
            Image copy = image;
            vectorFlipper.flip(copy, FlippingDirection_V::VERTICAL);
        };
        time_original_v = bench_ms(vector_fn, iterations);
    }

    // Benchmark LMUL variants for vertical flipping
    std::vector<std::pair<std::string, double>> lmul_results_v;
    
    // LMUL=1
    {
        auto lmul1_fn = [&]() {
            auto result = flip_vertical_LMUL<1>(image.pixelMatrix);
        };
        double time_m1 = bench_ms(lmul1_fn, iterations);
        lmul_results_v.push_back({"m1", time_m1});
    }
    
    // LMUL=2
    {
        auto lmul2_fn = [&]() {
            auto result = flip_vertical_LMUL<2>(image.pixelMatrix);
        };
        double time_m2 = bench_ms(lmul2_fn, iterations);
        lmul_results_v.push_back({"m2", time_m2});
    }
    
    // LMUL=4
    {
        auto lmul4_fn = [&]() {
            auto result = flip_vertical_LMUL<4>(image.pixelMatrix);
        };
        double time_m4 = bench_ms(lmul4_fn, iterations);
        lmul_results_v.push_back({"m4", time_m4});
    }
    
    // LMUL=8
    {
        auto lmul8_fn = [&]() {
            auto result = flip_vertical_LMUL<8>(image.pixelMatrix);
        };
        double time_m8 = bench_ms(lmul8_fn, iterations);
        lmul_results_v.push_back({"m8", time_m8});
    }

    // Print vertical flipping results
    std::cout << std::setw(15) << "Implementation" 
              << std::setw(12) << "Time (ms)" 
              << std::setw(12) << "Speedup"
              << std::setw(15) << "vs Scalar"
              << std::endl;
    std::cout << std::string(54, '-') << std::endl;
    
    std::cout << std::setw(15) << "Scalar" 
              << std::setw(12) << std::fixed << std::setprecision(3) << time_scalar_v
              << std::setw(12) << "1.00x"
              << std::setw(15) << "baseline"
              << std::endl;
              
    std::cout << std::setw(15) << "Original Vector" 
              << std::setw(12) << std::fixed << std::setprecision(3) << time_original_v
              << std::setw(12) << std::fixed << std::setprecision(2) << (time_scalar_v / time_original_v) << "x"
              << std::setw(15) << std::fixed << std::setprecision(2) << (time_scalar_v / time_original_v) << "x"
              << std::endl;

    for (const auto& result : lmul_results_v) {
        double speedup_vs_scalar = time_scalar_v / result.second;
        double speedup_vs_m1 = lmul_results_v[0].second / result.second;
        
        std::cout << std::setw(15) << ("LMUL " + result.first)
                  << std::setw(12) << std::fixed << std::setprecision(3) << result.second
                  << std::setw(12) << std::fixed << std::setprecision(2) << speedup_vs_m1 << "x"
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup_vs_scalar << "x"
                  << std::endl;
    }

    // ===== ANALYSIS =====
    std::cout << std::endl << "=== LMUL ANALYSIS ===" << std::endl;
    
    auto best_lmul_h = std::min_element(lmul_results_h.begin(), lmul_results_h.end(),
                                       [](const auto& a, const auto& b) { return a.second < b.second; });
    
    auto best_lmul_v = std::min_element(lmul_results_v.begin(), lmul_results_v.end(),
                                       [](const auto& a, const auto& b) { return a.second < b.second; });
    
    std::cout << "Best LMUL for horizontal flip: " << best_lmul_h->first 
              << " (" << std::fixed << std::setprecision(2) << (time_scalar_h / best_lmul_h->second) << "x vs scalar)" << std::endl;
              
    std::cout << "Best LMUL for vertical flip: " << best_lmul_v->first 
              << " (" << std::fixed << std::setprecision(2) << (time_scalar_v / best_lmul_v->second) << "x vs scalar)" << std::endl;

    std::cout << std::endl << "Horizontal flip LMUL efficiency:" << std::endl;
    for (size_t i = 0; i < lmul_results_h.size(); ++i) {
        double theoretical_speedup = (i == 0) ? 1.0 : std::pow(2, i);
        double actual_speedup = lmul_results_h[0].second / lmul_results_h[i].second;
        double efficiency = (actual_speedup / theoretical_speedup) * 100.0;
        
        std::cout << "  " << lmul_results_h[i].first << ": " 
                  << std::fixed << std::setprecision(1) << efficiency << "% efficiency" << std::endl;
    }

    std::cout << std::endl << "Vertical flip LMUL efficiency:" << std::endl;
    for (size_t i = 0; i < lmul_results_v.size(); ++i) {
        double theoretical_speedup = (i == 0) ? 1.0 : std::pow(2, i);
        double actual_speedup = lmul_results_v[0].second / lmul_results_v[i].second;
        double efficiency = (actual_speedup / theoretical_speedup) * 100.0;
        
        std::cout << "  " << lmul_results_v[i].first << ": " 
                  << std::fixed << std::setprecision(1) << efficiency << "% efficiency" << std::endl;
    }

    return 0;
} 