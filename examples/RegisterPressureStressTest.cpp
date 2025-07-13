#include "VectorTraits_LMUL.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

// Generic benchmark that runs `fn` once for warm-up and `iterations` times for timing
template <typename F>
double bench_ms(F fn, int iterations = 50) {
    fn();  // warm-up
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) fn();
    auto t1 = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();
    return total_ms / iterations;
}

// Conservative Gaussian filter - minimal register usage
template<int LMUL>
std::vector<std::vector<uint8_t>> gaussian_conservative(const std::vector<std::vector<uint8_t>>& img) {
    const int height = img.size();
    const int width = img[0].size();
    std::vector<std::vector<uint8_t>> result(height, std::vector<uint8_t>(width));
    
    using Traits = VectorTraits_LMUL<uint8_t, LMUL>;
    
    for (int i = 1; i < height - 1; ++i) {
        int j = 0;
        while (j < width - 2) {
            size_t vl = Traits::vsetvl(width - 2 - j);
            
            // Process one neighbor at a time - MINIMAL register usage
            auto sum = Traits::vle(&img[i][j + 1], vl);  // Center pixel
            sum = Traits::vmul_vx(sum, 4, vl);           // Weight center by 4
            
            // Add each neighbor one by one, reusing temporary registers
            auto neighbor = Traits::vle(&img[i-1][j], vl);
            sum = Traits::vsaddu_vv(sum, neighbor, vl);
            
            neighbor = Traits::vle(&img[i-1][j + 1], vl);
            auto weighted = Traits::vmul_vx(neighbor, 2, vl);
            sum = Traits::vsaddu_vv(sum, weighted, vl);
            
            neighbor = Traits::vle(&img[i-1][j + 2], vl);
            sum = Traits::vsaddu_vv(sum, neighbor, vl);
            
            neighbor = Traits::vle(&img[i][j], vl);
            weighted = Traits::vmul_vx(neighbor, 2, vl);
            sum = Traits::vsaddu_vv(sum, weighted, vl);
            
            neighbor = Traits::vle(&img[i][j + 2], vl);
            weighted = Traits::vmul_vx(neighbor, 2, vl);
            sum = Traits::vsaddu_vv(sum, weighted, vl);
            
            neighbor = Traits::vle(&img[i+1][j], vl);
            sum = Traits::vsaddu_vv(sum, neighbor, vl);
            
            neighbor = Traits::vle(&img[i+1][j + 1], vl);
            weighted = Traits::vmul_vx(neighbor, 2, vl);
            sum = Traits::vsaddu_vv(sum, weighted, vl);
            
            neighbor = Traits::vle(&img[i+1][j + 2], vl);
            sum = Traits::vsaddu_vv(sum, neighbor, vl);
            
            auto filtered = Traits::vmul_vx(sum, 16, vl);  // Approximate /16
            Traits::vse(&result[i][j + 1], filtered, vl);
            
            j += vl;
        }
    }
    
    return result;
}

// AGGRESSIVE Gaussian filter - MAXIMUM register pressure via unrolling
template<int LMUL>
std::vector<std::vector<uint8_t>> gaussian_register_stress(const std::vector<std::vector<uint8_t>>& img) {
    const int height = img.size();
    const int width = img[0].size();
    std::vector<std::vector<uint8_t>> result(height, std::vector<uint8_t>(width));
    
    using Traits = VectorTraits_LMUL<uint8_t, LMUL>;
    
    for (int i = 1; i < height - 1; ++i) {
        int j = 0;
        while (j < width - 2) {
            size_t vl = Traits::vsetvl(width - 2 - j);
            
            // FORCE MAXIMUM REGISTER PRESSURE:
            // Load ALL neighbors simultaneously - no register reuse!
            
            // 3x3 kernel loads (9 vector registers used)
            auto p00 = Traits::vle(&img[i-1][j], vl);     // Register 1
            auto p01 = Traits::vle(&img[i-1][j+1], vl);   // Register 2  
            auto p02 = Traits::vle(&img[i-1][j+2], vl);   // Register 3
            auto p10 = Traits::vle(&img[i][j], vl);       // Register 4
            auto p11 = Traits::vle(&img[i][j+1], vl);     // Register 5 (center)
            auto p12 = Traits::vle(&img[i][j+2], vl);     // Register 6
            auto p20 = Traits::vle(&img[i+1][j], vl);     // Register 7
            auto p21 = Traits::vle(&img[i+1][j+1], vl);   // Register 8
            auto p22 = Traits::vle(&img[i+1][j+2], vl);   // Register 9
            
            // Apply weights to ALL pixels simultaneously (9 more registers!)
            auto w00 = Traits::vmul_vx(p00, 1, vl);       // Register 10
            auto w01 = Traits::vmul_vx(p01, 2, vl);       // Register 11
            auto w02 = Traits::vmul_vx(p02, 1, vl);       // Register 12
            auto w10 = Traits::vmul_vx(p10, 2, vl);       // Register 13
            auto w11 = Traits::vmul_vx(p11, 4, vl);       // Register 14  
            auto w12 = Traits::vmul_vx(p12, 2, vl);       // Register 15
            auto w20 = Traits::vmul_vx(p20, 1, vl);       // Register 16
            auto w21 = Traits::vmul_vx(p21, 2, vl);       // Register 17
            auto w22 = Traits::vmul_vx(p22, 1, vl);       // Register 18
            
            // Partial sums (more registers!)
            auto sum_row0 = Traits::vsaddu_vv(w00, w01, vl);    // Register 19
            sum_row0 = Traits::vsaddu_vv(sum_row0, w02, vl);    // Reuse 19
            
            auto sum_row1 = Traits::vsaddu_vv(w10, w11, vl);    // Register 20
            sum_row1 = Traits::vsaddu_vv(sum_row1, w12, vl);    // Reuse 20
            
            auto sum_row2 = Traits::vsaddu_vv(w20, w21, vl);    // Register 21
            sum_row2 = Traits::vsaddu_vv(sum_row2, w22, vl);    // Reuse 21
            
            // Final sum and result
            auto total_sum = Traits::vsaddu_vv(sum_row0, sum_row1, vl);  // Register 22
            total_sum = Traits::vsaddu_vv(total_sum, sum_row2, vl);      // Reuse 22
            
            auto filtered = Traits::vmul_vx(total_sum, 16, vl);          // Register 23
            
            // Store result
            Traits::vse(&result[i][j + 1], filtered, vl);
            
            j += vl;
        }
    }
    
    return result;
}

// EXTREME register pressure test - unroll multiple pixels
template<int LMUL>
std::vector<std::vector<uint8_t>> gaussian_extreme_stress(const std::vector<std::vector<uint8_t>>& img) {
    const int height = img.size();
    const int width = img[0].size();
    std::vector<std::vector<uint8_t>> result(height, std::vector<uint8_t>(width));
    
    using Traits = VectorTraits_LMUL<uint8_t, LMUL>;
    
    for (int i = 1; i < height - 1; ++i) {
        int j = 0;
        while (j < width - 8) {  // Process 4 pixels simultaneously
            size_t vl = Traits::vsetvl(std::min(width - 8 - j, 8));
            
            // UNROLL 4 PIXELS WORTH OF GAUSSIAN FILTER
            // This should require 4 * 9 = 36 vector registers minimum!
            
            // Pixel 1 loads (9 registers)
            auto p1_00 = Traits::vle(&img[i-1][j], vl);
            auto p1_01 = Traits::vle(&img[i-1][j+1], vl);
            auto p1_02 = Traits::vle(&img[i-1][j+2], vl);
            auto p1_10 = Traits::vle(&img[i][j], vl);
            auto p1_11 = Traits::vle(&img[i][j+1], vl);
            auto p1_12 = Traits::vle(&img[i][j+2], vl);
            auto p1_20 = Traits::vle(&img[i+1][j], vl);
            auto p1_21 = Traits::vle(&img[i+1][j+1], vl);
            auto p1_22 = Traits::vle(&img[i+1][j+2], vl);
            
            // Pixel 2 loads (9 more registers = 18 total)
            auto p2_00 = Traits::vle(&img[i-1][j+1], vl);
            auto p2_01 = Traits::vle(&img[i-1][j+2], vl);
            auto p2_02 = Traits::vle(&img[i-1][j+3], vl);
            auto p2_10 = Traits::vle(&img[i][j+1], vl);
            auto p2_11 = Traits::vle(&img[i][j+2], vl);
            auto p2_12 = Traits::vle(&img[i][j+3], vl);
            auto p2_20 = Traits::vle(&img[i+1][j+1], vl);
            auto p2_21 = Traits::vle(&img[i+1][j+2], vl);
            auto p2_22 = Traits::vle(&img[i+1][j+3], vl);
            
            // Pixel 3 loads (9 more registers = 27 total)
            auto p3_00 = Traits::vle(&img[i-1][j+2], vl);
            auto p3_01 = Traits::vle(&img[i-1][j+3], vl);
            auto p3_02 = Traits::vle(&img[i-1][j+4], vl);
            auto p3_10 = Traits::vle(&img[i][j+2], vl);
            auto p3_11 = Traits::vle(&img[i][j+3], vl);
            auto p3_12 = Traits::vle(&img[i][j+4], vl);
            auto p3_20 = Traits::vle(&img[i+1][j+2], vl);
            auto p3_21 = Traits::vle(&img[i+1][j+3], vl);
            auto p3_22 = Traits::vle(&img[i+1][j+4], vl);
            
            // Pixel 4 loads (9 more registers = 36 total!)
            auto p4_00 = Traits::vle(&img[i-1][j+3], vl);
            auto p4_01 = Traits::vle(&img[i-1][j+4], vl);
            auto p4_02 = Traits::vle(&img[i-1][j+5], vl);
            auto p4_10 = Traits::vle(&img[i][j+3], vl);
            auto p4_11 = Traits::vle(&img[i][j+4], vl);
            auto p4_12 = Traits::vle(&img[i][j+5], vl);
            auto p4_20 = Traits::vle(&img[i+1][j+3], vl);
            auto p4_21 = Traits::vle(&img[i+1][j+4], vl);
            auto p4_22 = Traits::vle(&img[i+1][j+5], vl);
            
            // Process each pixel (more temporary registers!)
            auto sum1 = Traits::vmul_vx(p1_11, 4, vl);  // +4 registers
            sum1 = Traits::vsaddu_vv(sum1, p1_00, vl);
            sum1 = Traits::vsaddu_vv(sum1, Traits::vmul_vx(p1_01, 2, vl), vl);
            sum1 = Traits::vsaddu_vv(sum1, p1_02, vl);
            
            auto sum2 = Traits::vmul_vx(p2_11, 4, vl);  // +4 registers  
            sum2 = Traits::vsaddu_vv(sum2, p2_00, vl);
            sum2 = Traits::vsaddu_vv(sum2, Traits::vmul_vx(p2_01, 2, vl), vl);
            sum2 = Traits::vsaddu_vv(sum2, p2_02, vl);
            
            auto sum3 = Traits::vmul_vx(p3_11, 4, vl);  // +4 registers
            sum3 = Traits::vsaddu_vv(sum3, p3_00, vl);
            sum3 = Traits::vsaddu_vv(sum3, Traits::vmul_vx(p3_01, 2, vl), vl);
            sum3 = Traits::vsaddu_vv(sum3, p3_02, vl);
            
            auto sum4 = Traits::vmul_vx(p4_11, 4, vl);  // +4 registers
            sum4 = Traits::vsaddu_vv(sum4, p4_00, vl);
            sum4 = Traits::vsaddu_vv(sum4, Traits::vmul_vx(p4_01, 2, vl), vl);
            sum4 = Traits::vsaddu_vv(sum4, p4_02, vl);
            
            // Final results  
            auto result1 = Traits::vmul_vx(sum1, 16, vl);
            auto result2 = Traits::vmul_vx(sum2, 16, vl);
            auto result3 = Traits::vmul_vx(sum3, 16, vl);
            auto result4 = Traits::vmul_vx(sum4, 16, vl);
            
            // Store results
            Traits::vse(&result[i][j + 1], result1, vl);
            Traits::vse(&result[i][j + 2], result2, vl);
            Traits::vse(&result[i][j + 3], result3, vl);
            Traits::vse(&result[i][j + 4], result4, vl);
            
            j += 4;
        }
    }
    
    return result;
}

int main() {
    // Create test image
    const int height = 512;
    const int width = 512;
    std::vector<std::vector<uint8_t>> image(height, std::vector<uint8_t>(width));
    
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            image[i][j] = static_cast<uint8_t>((i + j) % 256);
        }
    }

    const int iterations = 50;

    std::cout << "=== RISC-V Register Pressure Stress Test ===" << std::endl;
    std::cout << "Testing if Spike simulator models finite vector registers" << std::endl;
    std::cout << "Image size: " << height << " x " << width << std::endl;
    std::cout << "Iterations: " << iterations << std::endl << std::endl;

    // Test register pressure with different algorithms
    std::vector<std::string> test_names = {
        "Conservative (minimal registers)",
        "Aggressive (high register usage)", 
        "EXTREME (36+ registers required)"
    };

    for (int lmul_exp = 0; lmul_exp <= 3; ++lmul_exp) {
        int lmul_val = 1 << lmul_exp;
        std::string lmul_str = "m" + std::to_string(lmul_val);
        
        std::cout << "=== LMUL=" << lmul_str << " ===" << std::endl;
        std::cout << "Available vector registers: " << (32 / lmul_val) << std::endl;
        
        std::vector<double> times;
        
        // Conservative test
        if (lmul_val == 1) {
            auto test_fn = [&]() { auto result = gaussian_conservative<1>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 2) {
            auto test_fn = [&]() { auto result = gaussian_conservative<2>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 4) {
            auto test_fn = [&]() { auto result = gaussian_conservative<4>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 8) {
            auto test_fn = [&]() { auto result = gaussian_conservative<8>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        }
        
        // Aggressive test
        if (lmul_val == 1) {
            auto test_fn = [&]() { auto result = gaussian_register_stress<1>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 2) {
            auto test_fn = [&]() { auto result = gaussian_register_stress<2>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 4) {
            auto test_fn = [&]() { auto result = gaussian_register_stress<4>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 8) {
            auto test_fn = [&]() { auto result = gaussian_register_stress<8>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        }
        
        // Extreme test
        if (lmul_val == 1) {
            auto test_fn = [&]() { auto result = gaussian_extreme_stress<1>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 2) {
            auto test_fn = [&]() { auto result = gaussian_extreme_stress<2>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 4) {
            auto test_fn = [&]() { auto result = gaussian_extreme_stress<4>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        } else if (lmul_val == 8) {
            auto test_fn = [&]() { auto result = gaussian_extreme_stress<8>(image); };
            times.push_back(bench_ms(test_fn, iterations));
        }
        
        // Print results
        for (size_t i = 0; i < test_names.size(); ++i) {
            std::cout << "  " << test_names[i] << ": " 
                      << std::fixed << std::setprecision(3) << times[i] << " ms" << std::endl;
        }
        
        // Analysis
        double pressure_penalty = times[2] / times[0];  // extreme vs conservative
        std::cout << "  Register pressure penalty: " << std::fixed << std::setprecision(2) 
                  << pressure_penalty << "x" << std::endl;
        
        std::cout << std::endl;
    }

    std::cout << "=== ANALYSIS ===" << std::endl;
    std::cout << "If Spike models finite registers correctly:" << std::endl;
    std::cout << "• LMUL=m8 should show SEVERE penalties for extreme test" << std::endl;
    std::cout << "• LMUL=m4 should show moderate penalties" << std::endl;
    std::cout << "• Conservative vs Extreme should show big differences" << std::endl;
    std::cout << std::endl;
    std::cout << "If penalties are minimal across all LMUL values," << std::endl;
    std::cout << "then Spike does NOT model register pressure!" << std::endl;

    return 0;
} 