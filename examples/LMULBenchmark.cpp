#include "ImageReader.hpp"
#include "ImageWriter.hpp"
#include "VectorTraits_LMUL.hpp"
#include "Image.hpp"
#include "ImageStatus.hpp"
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

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

struct BenchmarkResult {
    std::string operation;
    int lmul;
    const char* lmul_str;
    double time_ms;
    double throughput;
    double efficiency;  // Relative to m1
};

class LMULBenchmarkSuite {
private:
    std::vector<BenchmarkResult> results;
    std::vector<std::vector<uint8_t>> testImage;
    const int iterations = 100;
    ImageReader<uint8_t> reader;
    
public:
    bool initialize(const std::string& imagePath) {
        // Read the actual image file
        Image image;
        ImageStatus status = reader.readImage(imagePath, image);
        if (status != ImageStatus::SUCCESS) {
            std::cerr << "Failed to read image: " << static_cast<int>(status) << std::endl;
            return false;
        }
        
        // Copy the pixel matrix to our test image
        testImage = image.pixelMatrix;
        
        std::cout << "=== RISC-V Vector LMUL Performance Analysis ===" << std::endl;
        std::cout << "Image: " << imagePath << std::endl;
        std::cout << "Image size: " << testImage.size() 
                  << " x " << testImage[0].size() << std::endl;
        std::cout << "Iterations per test: " << iterations << std::endl;
        std::cout << "Testing LMUL values: m1, m2, m4, m8" << std::endl << std::endl;
        
        return true;
    }
    
    template<int LMUL>
    void benchmarkImageAddition() {
        using Traits = VectorTraits_uint8_LMUL<LMUL>;
        
        auto lambda = [&]() {
            simple_add_LMUL<uint8_t, LMUL>(testImage, testImage);
        };
        
        double time_ms = bench_ms(lambda, iterations);
        double pixels = testImage.size() * testImage[0].size();
        double throughput = (pixels / 1e6) / (time_ms / 1000.0);  // Megapixels per second
        
        results.push_back({
            "Image Addition",
            LMUL,
            Traits::lmul_str,
            time_ms,
            throughput,
            0.0  // Will calculate later
        });
    }
    
    template<int LMUL>
    void benchmarkHorizontalFlip() {
        using Traits = VectorTraits_uint8_LMUL<LMUL>;
        
        auto lambda = [&]() {
            simple_flip_horizontal_LMUL<uint8_t, LMUL>(testImage);
        };
        
        double time_ms = bench_ms(lambda, iterations);
        double pixels = testImage.size() * testImage[0].size();
        double throughput = (pixels / 1e6) / (time_ms / 1000.0);
        
        results.push_back({
            "Horizontal Flip",
            LMUL,
            Traits::lmul_str,
            time_ms,
            throughput,
            0.0
        });
    }
    
    template<int LMUL>
    void benchmarkBoxFilter() {
        using Traits = VectorTraits_uint8_LMUL<LMUL>;
        const int kernelSize = 5;
        
        auto lambda = [&]() {
            // Simplified box filter using LMUL traits
            const int height = testImage.size();
            const int width = testImage[0].size();
            std::vector<std::vector<uint8_t>> result(height, std::vector<uint8_t>(width));
            
            for (int i = 1; i < height - 1; ++i) {
                int j = 0;
                while (j < width - kernelSize) {
                    size_t vl = Traits::vsetvl(width - kernelSize - j);
                    
                    // Load center pixels
                    auto center = Traits::vle(&testImage[i][j + 2], vl);
                    
                    // Simple averaging (not full box filter for performance testing)
                    auto neighbors = Traits::vle(&testImage[i-1][j + 2], vl);
                    auto sum = Traits::vsaddu_vv(center, neighbors, vl);
                    
                    neighbors = Traits::vle(&testImage[i+1][j + 2], vl);
                    sum = Traits::vsaddu_vv(sum, neighbors, vl);
                    
                    // Simple divide by 3 using bit shifts
                    auto divided = Traits::vmul_vx(sum, 85, vl);  // 85/256 ≈ 1/3
                    
                    Traits::vse(&result[i][j + 2], divided, vl);
                    j += vl;
                }
            }
        };
        
        double time_ms = bench_ms(lambda, iterations);
        double pixels = testImage.size() * testImage[0].size();
        double throughput = (pixels / 1e6) / (time_ms / 1000.0);
        
        results.push_back({
            "Box Filter (5x5)",
            LMUL,
            Traits::lmul_str,
            time_ms,
            throughput,
            0.0
        });
    }
    
    void runAllBenchmarks() {
        std::cout << "Running Image Addition benchmarks..." << std::endl;
        benchmarkImageAddition<1>();
        benchmarkImageAddition<2>();
        benchmarkImageAddition<4>();
        benchmarkImageAddition<8>();
        
        std::cout << "Running Horizontal Flip benchmarks..." << std::endl;
        benchmarkHorizontalFlip<1>();
        benchmarkHorizontalFlip<2>();
        benchmarkHorizontalFlip<4>();
        benchmarkHorizontalFlip<8>();
        
        std::cout << "Running Box Filter benchmarks..." << std::endl;
        benchmarkBoxFilter<1>();
        benchmarkBoxFilter<2>();
        benchmarkBoxFilter<4>();
        benchmarkBoxFilter<8>();
        
        calculateEfficiencies();
    }
    
    void calculateEfficiencies() {
        std::vector<std::string> operations = {"Image Addition", "Horizontal Flip", "Box Filter (5x5)"};
        
        for (const auto& op : operations) {
            double m1_time = 0.0;
            
            // Find m1 time for this operation
            for (const auto& result : results) {
                if (result.operation == op && result.lmul == 1) {
                    m1_time = result.time_ms;
                    break;
                }
            }
            
            // Calculate efficiencies relative to m1
            for (auto& result : results) {
                if (result.operation == op && m1_time > 0) {
                    result.efficiency = m1_time / result.time_ms;
                }
            }
        }
    }
    
    void printResults() {
        std::cout << "\n=== LMUL PERFORMANCE COMPARISON ===" << std::endl;
        std::cout << std::setw(20) << "Operation" 
                  << std::setw(8) << "LMUL" 
                  << std::setw(12) << "Time (ms)" 
                  << std::setw(15) << "Throughput"
                  << std::setw(12) << "Speedup"
                  << std::endl;
        std::cout << std::setw(20) << "" 
                  << std::setw(8) << "" 
                  << std::setw(12) << "" 
                  << std::setw(15) << "(MPix/sec)"
                  << std::setw(12) << "vs m1"
                  << std::endl;
        std::cout << std::string(67, '-') << std::endl;
        
        for (const auto& result : results) {
            std::cout << std::setw(20) << result.operation
                      << std::setw(8) << result.lmul_str
                      << std::setw(12) << std::fixed << std::setprecision(3) << result.time_ms
                      << std::setw(15) << std::fixed << std::setprecision(2) << result.throughput
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.efficiency << "x"
                      << std::endl;
        }
    }
    
    void generateCSV(const std::string& filename) {
        std::ofstream file(filename);
        file << "Operation,LMUL,Time_ms,Throughput_MPix_sec,Speedup_vs_m1\n";
        
        for (const auto& result : results) {
            file << result.operation << ","
                 << result.lmul_str << ","
                 << std::fixed << std::setprecision(3) << result.time_ms << ","
                 << std::fixed << std::setprecision(2) << result.throughput << ","
                 << std::fixed << std::setprecision(2) << result.efficiency << "\n";
        }
        
        std::cout << "\nResults saved to: " << filename << std::endl;
        std::cout << "Use this CSV file to generate graphs in your preferred plotting tool." << std::endl;
    }
    
    void generatePythonScript(const std::string& scriptName) {
        std::ofstream script(scriptName);
        script << "#!/usr/bin/env python3\n";
        script << "import pandas as pd\n";
        script << "import matplotlib.pyplot as plt\n";
        script << "import numpy as np\n\n";
        script << "# Read the benchmark data\n";
        script << "df = pd.read_csv('lmul_benchmark_results.csv')\n\n";
        script << "# Create a simple bar plot\n";
        script << "plt.figure(figsize=(12, 8))\n";
        script << "operations = df['Operation'].unique()\n";
        script << "lmul_values = ['m1', 'm2', 'm4', 'm8']\n";
        script << "colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']\n\n";
        script << "x = np.arange(len(operations))\n";
        script << "width = 0.2\n\n";
        script << "for i, lmul in enumerate(lmul_values):\n";
        script << "    times = [df[(df['Operation'] == op) & (df['LMUL'] == lmul)]['Time_ms'].values[0] for op in operations]\n";
        script << "    plt.bar(x + i*width, times, width, label=lmul, color=colors[i])\n\n";
        script << "plt.xlabel('Operation')\n";
        script << "plt.ylabel('Time (ms)')\n";
        script << "plt.title('LMUL Performance Comparison')\n";
        script << "plt.xticks(x + width * 1.5, operations, rotation=45, ha='right')\n";
        script << "plt.legend()\n";
        script << "plt.grid(True, alpha=0.3)\n";
        script << "plt.tight_layout()\n";
        script << "plt.savefig('lmul_performance_analysis.png', dpi=300, bbox_inches='tight')\n";
        script << "plt.show()\n";
        
        std::cout << "\nPython plotting script generated: " << scriptName << std::endl;
        std::cout << "Run 'python3 " << scriptName << "' to generate performance graphs." << std::endl;
    }
};

int main() {
    LMULBenchmarkSuite suite;
    
    if (!suite.initialize("barb.512.pgm")) {
        return 1;
    }
    
    suite.runAllBenchmarks();
    suite.printResults();
    suite.generateCSV("lmul_benchmark_results.csv");
    suite.generatePythonScript("plot_lmul_results.py");
    
    std::cout << "\n=== ANALYSIS COMPLETE ===" << std::endl;
    std::cout << "1. Review the console output above for immediate results" << std::endl;
    std::cout << "2. Open lmul_benchmark_results.csv for detailed data" << std::endl;
    std::cout << "3. Run 'python3 plot_lmul_results.py' to generate graphs" << std::endl;
    std::cout << "\nThis analysis will help determine optimal LMUL values for different operations." << std::endl;
    
    return 0;
} 