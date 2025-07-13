#include "BoxFilter_Vector_Template.hpp"
#include "VectorTraits.hpp"
// Define vector traits for different data types (u8/u16)

// Templated zeroPad function
template <typename T>
static vector<vector<T>> __riscv_zeroPadImage(const vector<vector<T>>& image, int padSize) {
    if (image.empty() || image[0].empty()) return {};

    int H = image.size();
    int W = image[0].size();
    int PH = H + 2 * padSize;
    int PW = W + 2 * padSize;
    vector<vector<T>> padded(PH, vector<T>(PW, 0));

    for (int i = 0; i < H; ++i) {
        // Use pointers for clarity and efficiency
        const T* src_row_ptr = &image[i][0];
        T* dst_row_ptr = &padded[i + padSize][padSize];
        
        int n = W;
        while (n > 0) {
            // CORRECT: Use VectorTraits for vsetvl, vle, and vse
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


// Templated BoxFilter function
template<typename T>
vector<vector<T>> __riscv_BoxFilter(
    const vector<vector<T>> &inputImg,
    int kernelSize)
{
    if (inputImg.empty() || inputImg[0].empty()) {
        throw invalid_argument("Image is empty");
    }

    int rows = inputImg.size();
    int cols = inputImg[0].size();
    int border = kernelSize / 2;

    if (kernelSize > rows || kernelSize > cols || (kernelSize % 2) == 0) {
        throw invalid_argument("Invalid kernel size");
    }

    // Output, padded input, and temporary buffer
    vector<vector<T>> outputImg(rows, vector<T>(cols, 0));
    vector<vector<T>> padded = __riscv_zeroPadImage(inputImg, kernelSize);
    vector<vector<T>> tempImg = padded;

// --- Horizontal pass (1×K) ---
for (int i = 0; i < rows; ++i) {
    int y = i + border;
    int j = 0;
    while (j < cols) {
        // set vl once per chunk
        size_t vl = VectorTraits<T>::vsetvl(cols - j);

        auto zero     = VectorTraits<T>::vmv_v_x(   0, vl);
        auto vsum     = VectorTraits<T>::vmv_v_x_wide(0, vl);

        // accumulate the K horizontal taps
        for (int k = -border; k <= border; ++k) {
            const T* ptr = &padded[y][j + border + k];
            auto v       = VectorTraits<T>::vle(ptr, vl);
            auto vwide   = VectorTraits<T>::wadd_zero(v, vl);
            vsum         = VectorTraits<T>::vadd_vv_wide(vsum, vwide, vl);
        }

        // divide, narrow, store
        auto vavg        = VectorTraits<T>::divu(vsum, kernelSize, vl);
        auto vavg_narrow = VectorTraits<T>::vnclipu(vavg, 0, vl);
        VectorTraits<T>::vse(&tempImg[y][j + border], vavg_narrow, vl);

        j += vl;
    }
}

// --- Vertical pass (K×1) ---
for (int i = 0; i < rows; ++i) {
    int y0 = i + border;
    int j  = 0;
    while (j < cols) {
        // 1) set vl once per chunk
        size_t vl = VectorTraits<T>::vsetvl(cols - j);

        // 2) hoist zero-vector & initial sum
        auto zero = VectorTraits<T>::vmv_v_x(   0, vl);
        auto vsum = VectorTraits<T>::vmv_v_x_wide(0, vl);

        // 3) accumulate the K vertical taps
        for (int k = -border; k <= border; ++k) {
            const T* ptr = &tempImg[y0 + k][j + border];
            auto v       = VectorTraits<T>::vle(ptr, vl);
            auto vwide   = VectorTraits<T>::wadd_zero(v, vl);
            vsum         = VectorTraits<T>::vadd_vv_wide(vsum, vwide, vl);
        }

        // 4) divide, narrow, store
        auto vavg        = VectorTraits<T>::divu(vsum, kernelSize, vl);
        auto vavg_narrow = VectorTraits<T>::vnclipu(vavg, 0, vl);
        VectorTraits<T>::vse(&outputImg[i][j], vavg_narrow, vl);

        j += vl;
    }
}

    return outputImg;
}

template vector<vector<uint8_t>> __riscv_zeroPadImage<uint8_t>(
    const vector<vector<uint8_t>>&,
    int);
template vector<vector<uint8_t>> __riscv_BoxFilter<uint8_t>(
    const vector<vector<uint8_t>>&,
    int);
template vector<vector<uint16_t>> __riscv_zeroPadImage<uint16_t>(
        const vector<vector<uint16_t>>&,
        int);
template vector<vector<uint16_t>> __riscv_BoxFilter<uint16_t>(
        const vector<vector<uint16_t>>&,
        int);
