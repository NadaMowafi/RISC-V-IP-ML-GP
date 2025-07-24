#include "Gaussian_Vector.hpp"
#include <vector>
#include <cmath>
#include <cstdint>
#include <riscv_vector.h>
#include <algorithm>
#include <type_traits>

using std::vector;

// 1D Gaussian Kernel (double precision, normalized)
// vector<double> __riscv_generateGaussianKernel1D(int kernelSize, double sigma) {
//     if (kernelSize % 2 == 0) kernelSize += 1;
//     int half = kernelSize / 2;

//     vector<double> kernel(kernelSize);
//     double twoSigmaSq = 2.0 * sigma * sigma;
//     double sum = 0.0;

//     for (int i = 0; i < kernelSize; ++i) {
//         int x = i - half;
//         double v = std::exp(-(x * x) / twoSigmaSq);
//         kernel[i] = v;
//         sum += v;
//     }
//     for (auto &v : kernel) v /= sum;
//     return kernel;
// }

// Zero padding function (templated)
// template <typename T>
// static vector<vector<T>> __riscv_zeroPadImage(const vector<vector<T>>& image, int padSize) {
//     if (image.empty() || image[0].empty()) return {};

//     int H = image.size(), W = image[0].size();
//     int PH = H + 2 * padSize, PW = W + 2 * padSize;
//     vector<vector<T>> padded(PH, vector<T>(PW, 0));

//     for (int i = 0; i < H; ++i) {
//         for (int j = 0; j < W; j += __riscv_vsetvl_e64m4(W - j)) {
//             size_t vl = __riscv_vsetvl_e64m4(W - j);
//             // load/store based on T
//             // solution for 8, 16, 32, 64 bit types that made the compiler happy :)
//             // this is a bit of a hack, but it works
//             if constexpr (std::is_same_v<T, uint8_t>) {
//                 vuint8m4_t v = __riscv_vle8_v_u8m4(&image[i][j], vl);
//                 __riscv_vse8_v_u8m4(&padded[i+padSize][j+padSize], v, vl);
//             } else if constexpr (std::is_same_v<T, uint16_t>) {
//                 vuint16m4_t v = __riscv_vle16_v_u16m4(&image[i][j], vl);
//                 __riscv_vse16_v_u16m4(&padded[i+padSize][j+padSize], v, vl);
//             } else if constexpr (std::is_same_v<T, uint32_t>) {
//                 vuint32m4_t v = __riscv_vle32_v_u32m4(&image[i][j], vl);
//                 __riscv_vse32_v_u32m4(&padded[i+padSize][j+padSize], v, vl);
//             } else if constexpr (std::is_same_v<T, uint64_t>) {
//                 vuint64m4_t v = __riscv_vle64_v_u64m4(&image[i][j], vl);
//                 __riscv_vse64_v_u64m4(&padded[i+padSize][j+padSize], v, vl);
//             } else {
//                 // fallback scalar copy
//                 for (size_t k = 0; k < vl; ++k)
//                     padded[i+padSize][j+padSize+k] = image[i][j+k];
//             }
//         }
//     }
//     return padded;
// }

// // Separable convolution with zero padding and double-precision kernel
// template <typename T>
// vector<vector<T>> __riscv_applyGaussianFilterSeparable(
//     const vector<vector<T>>& image,
//     int kernelSize,
//     double sigma)
// {
//     if (image.empty() || image[0].empty()) return {};

//     int H = image.size(), W = image[0].size();
//     if (kernelSize <= 0) kernelSize = (static_cast<int>(std::ceil(sigma * 6)) | 1);
//     int half = kernelSize / 2;

//     // build double kernel
//     auto kernel1D = __riscv_generateGaussianKernel1D(kernelSize, sigma);

//     // pad image
//     auto padded = __riscv_zeroPadImage(image, half);
//     int PH = padded.size(), PW = padded[0].size();

//     // horizontal pass intermediate doubles
//     vector<vector<double>> interm(PH, vector<double>(PW, 0.0));
//     for (int i = 0; i < PH; ++i) {
//         for (int j = half; j < PW - half; j += __riscv_vsetvl_e64m4(PW - half - j)) {
//             size_t vl = __riscv_vsetvl_e64m4(PW - half - j);
//             vfloat64m4_t vsum = __riscv_vfmv_v_f_f64m4(0.0, vl);

//             for (int k = 0; k < kernelSize; ++k) {
//                 int col = j - half + k;
//                 // load block into double vector
//                 alignas(8) double tmp[vl];
//                 for (size_t l = 0; l < vl; ++l)
//                     tmp[l] = static_cast<double>(padded[i][col + l]);

//                 vfloat64m4_t vimg = __riscv_vle64_v_f64m4(tmp, vl);
//                 vsum = __riscv_vfadd_vv_f64m4(
//                     vsum,
//                     __riscv_vfmul_vf_f64m4(vimg, kernel1D[k], vl),
//                     vl
//                 );
//             }

//             // store intermediate
//             alignas(8) double out_tmp[vl];
//             __riscv_vse64_v_f64m4(out_tmp, vsum, vl);
//             for (size_t l = 0; l < vl; ++l)
//                 interm[i][j + l] = out_tmp[l];
//         }
//     }

//     // vertical pass back to T
//     vector<vector<T>> output(H, vector<T>(W, 0));
//     for (int j = half; j < PW - half; ++j) {
//         for (int i = half; i < PH - half; i += __riscv_vsetvl_e64m4(PH - half - i)) {
//             size_t vl = __riscv_vsetvl_e64m4(PH - half - i);
//             vfloat64m4_t vsum = __riscv_vfmv_v_f_f64m4(0.0, vl);

//             for (int k = 0; k < kernelSize; ++k) {
//                 int row = i - half + k;
//                 // load block
//                 alignas(8) double tmp[vl];
//                 for (size_t l = 0; l < vl; ++l)
//                     tmp[l] = interm[row + l][j];

//                 vfloat64m4_t vimg = __riscv_vle64_v_f64m4(tmp, vl);
//                 vsum = __riscv_vfadd_vv_f64m4(
//                     vsum,
//                     __riscv_vfmul_vf_f64m4(vimg, kernel1D[k], vl),
//                     vl
//                 );
//             }

//             // write back
//             alignas(8) double out_tmp[vl];
//             __riscv_vse64_v_f64m4(out_tmp, vsum, vl);
//             for (size_t l = 0; l < vl && (i - half + l) < H; ++l)
//                 output[i - half + l][j - half] = static_cast<T>(out_tmp[l]);
//         }
//     }

//     return output;
// }

// // instantiate for common types
// template vector<vector<uint8_t>>  __riscv_applyGaussianFilterSeparable<uint8_t>(const vector<vector<uint8_t>>&, int, double);
// template vector<vector<uint16_t>> __riscv_applyGaussianFilterSeparable<uint16_t>(const vector<vector<uint16_t>>&, int, double);
// template vector<vector<uint32_t>> __riscv_applyGaussianFilterSeparable<uint32_t>(const vector<vector<uint32_t>>&, int, double);
// template vector<vector<uint64_t>> __riscv_applyGaussianFilterSeparable<uint64_t>(const vector<vector<uint64_t>>&, int, double);
// template vector<vector<double>>   __riscv_applyGaussianFilterSeparable<double>(const vector<vector<double>>&, int, double);

