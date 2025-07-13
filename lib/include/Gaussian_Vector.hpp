#pragma once

#include <vector>
#include <riscv_vector.h>

template <typename T>
std::vector<std::vector<T>> __riscv_applyGaussianFilterSeparable(
    const std::vector<std::vector<T>>& image,
    int kernelSize,
    double sigma);
    
std::vector<double> __riscv_generateGaussianKernel1D(int N, double sigma);

// Explicit template declarations for supported types
extern template std::vector<std::vector<uint8_t>> __riscv_applyGaussianFilterSeparable(
    const std::vector<std::vector<uint8_t>>&, int, double);

extern template std::vector<std::vector<uint16_t>> __riscv_applyGaussianFilterSeparable(
    const std::vector<std::vector<uint16_t>>&, int, double);
