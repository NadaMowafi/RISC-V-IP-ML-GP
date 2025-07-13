#include "ImageReader.hpp"
#include "ImageWriter.hpp"
#include "Flipping.hpp"
#include "Flipping_Vector.hpp"
#include "Image.hpp"
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

    const int iterations = 100;

    // Read input image once
    ImageStatus status = reader.readImage("barb.512.pgm", image);
    if (status != ImageStatus::SUCCESS) {
        std::cerr << "Failed to read image: " << static_cast<int>(status) << std::endl;
        return 1;
    }

    std::cout << "=== Image Flipping Benchmark ===" << std::endl;
    std::cout << "Image size: " << image.pixelMatrix.size() << " x " << image.pixelMatrix[0].size() << std::endl;
    std::cout << "Iterations: " << iterations << std::endl << std::endl;

    // Create flipper objects
    ImageFlipper<uint8_t> scalarFlipper;
    ImageFlipper_V<uint8_t> vectorFlipper;

    // Test Horizontal Flipping
    {
        Image scalarOut = image;
        Image vectorOut = image;

        auto scalar_fn = [&]() {
            scalarFlipper.flip(scalarOut, FlippingDirection::HORIZONTAL);
        };
        auto vector_fn = [&]() {
            vectorFlipper.flip(vectorOut, FlippingDirection_V::HORIZONTAL);
        };

        double time_scalar = bench_ms(scalar_fn, iterations);
        double time_vector = bench_ms(vector_fn, iterations);
        
        std::cout << "--- Horizontal Flipping ---" << std::endl;
        std::cout << "Average scalar time  : " << time_scalar << " ms" << std::endl;
        std::cout << "Average vector time  : " << time_vector << " ms" << std::endl;
        std::cout << "Speedup (time)       : " << (time_scalar / time_vector) << "x" << std::endl << std::endl;
    }

    // Test Vertical Flipping
    {
        Image scalarOut = image;
        Image vectorOut = image;

        auto scalar_fn = [&]() {
            scalarFlipper.flip(scalarOut, FlippingDirection::VERTICAL);
        };
        auto vector_fn = [&]() {
            vectorFlipper.flip(vectorOut, FlippingDirection_V::VERTICAL);
        };

        double time_scalar = bench_ms(scalar_fn, iterations);
        double time_vector = bench_ms(vector_fn, iterations);
        
        std::cout << "--- Vertical Flipping ---" << std::endl;
        std::cout << "Average scalar time  : " << time_scalar << " ms" << std::endl;
        std::cout << "Average vector time  : " << time_vector << " ms" << std::endl;
        std::cout << "Speedup (time)       : " << (time_scalar / time_vector) << "x" << std::endl << std::endl;
    }

    return 0;
}
