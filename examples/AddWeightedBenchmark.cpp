#include "ImageReader.hpp"
#include "ImageWriter.hpp"
#include "Add.hpp"
#include "AddRVV.hpp"
#include <chrono>
#include <iostream>
#include <cstdint>

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

int main() {
    ImageReader reader;
    ImageWriter writer;
    Image image1, image2;

    const int iterations = 100;

    // Read input images once
    ImageStatus status1 = reader.readImage("barb.512.pgm", image1);
    if (status1 != ImageStatus::SUCCESS) {
        std::cerr << "Failed to read first image: " << static_cast<int>(status1) << std::endl;
        return 1;
    }

    // Use the same image for blending (could create variations if needed)
    image2 = image1;

    std::cout << "=== Weighted Addition Benchmark ===" << std::endl;
    std::cout << "Image size: " << image1.pixelMatrix.size() << " x " << image1.pixelMatrix[0].size() << std::endl;
    std::cout << "Iterations: " << iterations << std::endl << std::endl;

    // Test Alpha Blending (0.7 * img1 + 0.3 * img2 + 10)
    {
        std::vector<std::vector<uint8_t>> scalarOut, vectorOut;
        const float alpha = 0.7f;
        const float beta = 0.3f;
        const float gamma = 10.0f;

        auto scalar_fn = [&]() {
            scalarOut = addWeighted(
                image1.pixelMatrix, alpha,
                image2.pixelMatrix, beta,
                gamma, CLIPPING::SATURATION);
        };
        auto vector_fn = [&]() {
            vectorOut = __riscv_AddWeighted(
                image1.pixelMatrix, alpha,
                image2.pixelMatrix, beta,
                gamma, CLIPPING_V::SATURATION);
        };

        double time_scalar = bench_ms(scalar_fn, iterations);
        double time_vector = bench_ms(vector_fn, iterations);
        
        std::cout << "--- Alpha Blending (0.7*img1 + 0.3*img2 + 10) ---" << std::endl;
        std::cout << "Average scalar time  : " << time_scalar << " ms" << std::endl;
        std::cout << "Average vector time  : " << time_vector << " ms" << std::endl;
        std::cout << "Speedup (time)       : " << (time_scalar / time_vector) << "x" << std::endl << std::endl;
    }

    // Test Different Weights (0.5 * img1 + 0.5 * img2 + 0)
    {
        std::vector<std::vector<uint8_t>> scalarOut, vectorOut;
        const float alpha = 0.5f;
        const float beta = 0.5f;
        const float gamma = 0.0f;

        auto scalar_fn = [&]() {
            scalarOut = addWeighted(
                image1.pixelMatrix, alpha,
                image2.pixelMatrix, beta,
                gamma, CLIPPING::SATURATION);
        };
        auto vector_fn = [&]() {
            vectorOut = __riscv_AddWeighted(
                image1.pixelMatrix, alpha,
                image2.pixelMatrix, beta,
                gamma, CLIPPING_V::SATURATION);
        };

        double time_scalar = bench_ms(scalar_fn, iterations);
        double time_vector = bench_ms(vector_fn, iterations);
        
        std::cout << "--- Equal Blending (0.5*img1 + 0.5*img2) ---" << std::endl;
        std::cout << "Average scalar time  : " << time_scalar << " ms" << std::endl;
        std::cout << "Average vector time  : " << time_vector << " ms" << std::endl;
        std::cout << "Speedup (time)       : " << (time_scalar / time_vector) << "x" << std::endl << std::endl;
    }

    // Test with WRAP clipping
    {
        std::vector<std::vector<uint8_t>> scalarOut, vectorOut;
        const float alpha = 1.2f;
        const float beta = 0.8f;
        const float gamma = 50.0f;

        auto scalar_fn = [&]() {
            scalarOut = addWeighted(
                image1.pixelMatrix, alpha,
                image2.pixelMatrix, beta,
                gamma, CLIPPING::WRAP);
        };
        auto vector_fn = [&]() {
            vectorOut = __riscv_AddWeighted(
                image1.pixelMatrix, alpha,
                image2.pixelMatrix, beta,
                gamma, CLIPPING_V::WRAP);
        };

        double time_scalar = bench_ms(scalar_fn, iterations);
        double time_vector = bench_ms(vector_fn, iterations);
        
        std::cout << "--- Overflow Blending with WRAP (1.2*img1 + 0.8*img2 + 50) ---" << std::endl;
        std::cout << "Average scalar time  : " << time_scalar << " ms" << std::endl;
        std::cout << "Average vector time  : " << time_vector << " ms" << std::endl;
        std::cout << "Speedup (time)       : " << (time_scalar / time_vector) << "x" << std::endl << std::endl;
    }

    return 0;
}
