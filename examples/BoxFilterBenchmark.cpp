#include "ImageReader.hpp"
#include "ImageWriter.hpp"
#include "BoxFilter.hpp"
#include "BoxFilter_Vector_Template.hpp"
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
    Image image;

    const int kernelSize = 5;
    const int iterations = 100;

    // Read input image once
    ImageStatus status = reader.readImage("barb.512.pgm", image);
    if (status != ImageStatus::SUCCESS) {
        std::cerr << "Failed to read image: " << static_cast<int>(status) << std::endl;
        return 1;
    }

    // Prepare output containers (copy metadata)
    Image scalarOut = image;
    Image vectorOut = image;

    // Lambdas for scalar and vector filtering
    auto scalar_fn = [&]() {
        scalarOut.pixelMatrix = BoxFilter().applyBoxFilterSlidingGrey(
            image.pixelMatrix, kernelSize);
    };
    auto vector_fn = [&]() {
        vectorOut.pixelMatrix = __riscv_BoxFilter<uint8_t>(
            image.pixelMatrix, kernelSize);
    };

    // Benchmark and report average times
    double time_scalar = bench_ms(scalar_fn, iterations);
    double time_vector = bench_ms(vector_fn, iterations);
    std::cout << "Average scalar time  : " << time_scalar << " ms" << std::endl;
    std::cout << "Average vector time  : " << time_vector << " ms" << std::endl;
    std::cout << "Speedup (time)       : " << (time_scalar / time_vector) << "x" << std::endl;

    return 0;
}
