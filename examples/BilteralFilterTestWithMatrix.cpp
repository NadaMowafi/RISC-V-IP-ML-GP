#include "BilateralFilter.hpp"
#include <iostream>
#include <vector>
#include <iomanip> 

int main() {
    // Define a 9x9 test matrix (grayscale image)
    std::vector<std::vector<uint8_t>> testMatrix = {
        {10, 10, 10, 10, 10, 10, 10, 10, 10},
        {10, 20, 20, 20, 20, 20, 20, 20, 10},
        {10, 20, 30, 30, 30, 30, 30, 20, 10},
        {10, 20, 30, 40, 40, 40, 30, 20, 10},
        {10, 20, 30, 40, 50, 40, 30, 20, 10},
        {10, 20, 30, 40, 40, 40, 30, 20, 10},
        {10, 20, 30, 30, 30, 30, 30, 20, 10},
        {10, 20, 20, 20, 20, 20, 20, 20, 10},
        {10, 10, 10, 10, 10, 10, 10, 10, 10}
    };

    // Bilateral filter parameters
    int kernelSize = 5;          // Kernel size
    double sigmaSpatial = 2.0;   // Spatial sigma
    double sigmaIntensity = 10.0; // Intensity sigma

    // Apply the bilateral filter
    std::vector<std::vector<uint8_t>> filteredMatrix = BilateralFilter::apply(
        testMatrix,
        kernelSize,
        sigmaSpatial,
        sigmaIntensity
    );


    // Print the filtered matrix
    std::cout << "\nFiltered Matrix:" << std::endl;
    for (const auto& row : filteredMatrix) {
        for (const auto& pixel : row) {
            std::cout << std::setw(3) << static_cast<int>(pixel) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}