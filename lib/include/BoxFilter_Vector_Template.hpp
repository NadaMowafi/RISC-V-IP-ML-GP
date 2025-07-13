#ifndef BOXFILTER_VECTOR_TEMPLATE_HPP
#define BOXFILTER_VECTOR_TEMPLATE_HPP

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <riscv_vector.h>
using namespace std;

template<typename T>
struct VectorTraits;

// Declarations
template<typename T>
vector<vector<T>> __riscv_zeroPadImage(
    const vector<vector<T>> &inputImg,
    int kernelSize);

template<typename T>
vector<vector<T>> __riscv_BoxFilter(
    const vector<vector<T>> &inputImg,
    int kernelSize);

extern template vector<vector<uint8_t>> __riscv_zeroPadImage<uint8_t>(
    const vector<vector<uint8_t>> &,
    int);
extern template vector<vector<uint8_t>> __riscv_BoxFilter<uint8_t>(
    const vector<vector<uint8_t>> &,
    int);

extern template vector<vector<uint16_t>> __riscv_zeroPadImage<uint16_t>(
    const vector<vector<uint16_t>> &,
    int);
extern template vector<vector<uint16_t>> __riscv_BoxFilter<uint16_t>(
    const vector<vector<uint16_t>> &,
    int);

// extern template vector<vector<uint32_t>> __riscv_zeroPadImage<uint32_t>(
//         const vector<vector<uint32_t>>&,
//         int);
//     extern template vector<vector<uint32_t>> __riscv_BoxFilter<uint32_t>(
//         const vector<vector<uint32_t>>&,
//         int);
    


    
 #endif // BOXFILTER_VECTOR_TEMPLATE_HPP