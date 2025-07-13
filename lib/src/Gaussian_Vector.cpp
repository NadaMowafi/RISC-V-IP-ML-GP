#include "Gaussian_Vector.hpp"
#include "VectorTraits.hpp"
#include <cmath>
#include <type_traits>

std::vector<double> __riscv_generateGaussianKernel1D(int N, double sigma) {
    if ((N & 1) == 0) ++N;
    int half = N >> 1;
    double twoSig2 = 2.0 * sigma * sigma;
    std::vector<double> kernel(N);

    double sum = 0.0;
    for (int i = -half; i <= half; ++i) {
        kernel[i + half] = std::exp(-(i * i) / twoSig2);
        sum += kernel[i + half];
    }

    // Normalize
    for (int i = 0; i < N; ++i) {
        kernel[i] /= sum;
    }
    return kernel;
}

template <typename T>
static std::vector<std::vector<T>> __riscv_zeroPadImage(const std::vector<std::vector<T>>& image, int padSize) {
    if (image.empty() || image[0].empty()) return {};

    int H = image.size();
    int W = image[0].size();
    int PH = H + 2 * padSize;
    int PW = W + 2 * padSize;
    std::vector<std::vector<T>> padded(PH, std::vector<T>(PW, 0));

    for (int i = 0; i < H; ++i) {
        // Use pointers for clarity and efficiency
        const T* src_row_ptr = &image[i][0];
        T* dst_row_ptr = &padded[i + padSize][padSize];
        
        int n = W;
        while (n > 0) {
            size_t vl = VectorTraits<T>::vsetvl(n);
            typename VectorTraits<T>::vec_type v = VectorTraits<T>::vle(src_row_ptr, vl);
            VectorTraits<T>::vse(dst_row_ptr, v, vl);

            src_row_ptr += vl;
            dst_row_ptr += vl;
            n -= vl;
        }
    }
    return padded;
}

template <typename T>
std::vector<std::vector<T>> __riscv_applyGaussianFilterSeparable(
    const std::vector<std::vector<T>>& image,
    int kernelSize,
    double sigma)
{
    if (image.empty() || image[0].empty()) return {};

    int H = image.size(), W = image[0].size();
    if (kernelSize <= 0) kernelSize = (static_cast<int>(std::ceil(sigma * 6)) | 1);
    int half = kernelSize / 2;

    auto kernel1D = __riscv_generateGaussianKernel1D(kernelSize, sigma);
    auto padded = __riscv_zeroPadImage(image, half);

    // Convert kernel to fixed-point for integer arithmetic
    // Use different scale factors based on data type to maintain precision
    int SCALE_FACTOR;
    if constexpr (std::is_same_v<T, uint8_t>) {
        SCALE_FACTOR = 1024;  // 10-bit precision for 8-bit data
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        SCALE_FACTOR = 4096;  // 12-bit precision for 16-bit data
    } else {
        SCALE_FACTOR = 1024;  // Default fallback
    }
    
    std::vector<int> kernel_fixed(kernelSize);
    for (int i = 0; i < kernelSize; ++i) {
        kernel_fixed[i] = static_cast<int>(kernel1D[i] * SCALE_FACTOR + 0.5);
    }

    // Output and temporary buffer
    std::vector<std::vector<T>> outputImg(H, std::vector<T>(W, 0));
    std::vector<std::vector<T>> tempImg = padded;

    // Horizontal pass using vector traits (similar to BoxFilter)
    for (int i = 0; i < H; ++i) {
        int y = i + half;
        int j = 0;
        while (j < W) {
            // Set vl once per chunk
            size_t vl = VectorTraits<T>::vsetvl(W - j);

            // Initialize accumulator for wide arithmetic
            auto vsum = VectorTraits<T>::vmv_v_x_wide(0, vl);

            // Accumulate weighted horizontal taps
            for (int k = 0; k < kernelSize; ++k) {
                const T* ptr = &padded[y][j + half + k - half];
                auto v = VectorTraits<T>::vle(ptr, vl);
                auto vwide = VectorTraits<T>::wadd_zero(v, vl);
                
                // Multiply by kernel weight and accumulate
                if constexpr (std::is_same_v<T, uint8_t>) {
                    vwide = __riscv_vmul_vx_u16m2(vwide, kernel_fixed[k], vl);
                } else if constexpr (std::is_same_v<T, uint16_t>) {
                    vwide = __riscv_vmul_vx_u32m2(vwide, kernel_fixed[k], vl);
            }
                
                vsum = VectorTraits<T>::vadd_vv_wide(vsum, vwide, vl);
            }

            // Divide by scale factor, narrow, store
            auto vavg = VectorTraits<T>::divu(vsum, SCALE_FACTOR, vl);
            auto vavg_narrow = VectorTraits<T>::vnclipu(vavg, 0, vl);
            VectorTraits<T>::vse(&tempImg[y][j + half], vavg_narrow, vl);

            j += vl;
        }
    }

    // Vertical pass using vector traits
    for (int i = 0; i < H; ++i) {
        int y0 = i + half;
        int j = 0;
        while (j < W) {
            // Set vl once per chunk
            size_t vl = VectorTraits<T>::vsetvl(W - j);

            // Initialize accumulator for wide arithmetic
            auto vsum = VectorTraits<T>::vmv_v_x_wide(0, vl);

            // Accumulate weighted vertical taps
            for (int k = 0; k < kernelSize; ++k) {
                const T* ptr = &tempImg[y0 + k - half][j + half];
                auto v = VectorTraits<T>::vle(ptr, vl);
                auto vwide = VectorTraits<T>::wadd_zero(v, vl);
                
                // Multiply by kernel weight and accumulate
                if constexpr (std::is_same_v<T, uint8_t>) {
                    vwide = __riscv_vmul_vx_u16m2(vwide, kernel_fixed[k], vl);
                } else if constexpr (std::is_same_v<T, uint16_t>) {
                    vwide = __riscv_vmul_vx_u32m2(vwide, kernel_fixed[k], vl);
            }
                
                vsum = VectorTraits<T>::vadd_vv_wide(vsum, vwide, vl);
            }

            // Divide by scale factor, narrow, store
            auto vavg = VectorTraits<T>::divu(vsum, SCALE_FACTOR, vl);
            auto vavg_narrow = VectorTraits<T>::vnclipu(vavg, 0, vl);
            VectorTraits<T>::vse(&outputImg[i][j], vavg_narrow, vl);

            j += vl;
        }
    }

    return outputImg;
}

// Explicit instantiation for uint8_t and uint16_t
template std::vector<std::vector<uint8_t>> __riscv_applyGaussianFilterSeparable(
    const std::vector<std::vector<uint8_t>>&, int, double);

template std::vector<std::vector<uint16_t>> __riscv_applyGaussianFilterSeparable(
    const std::vector<std::vector<uint16_t>>&, int, double);
