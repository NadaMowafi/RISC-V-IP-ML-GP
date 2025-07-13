#ifndef ROTATE_CPP_V
#define ROTATE_CPP_V

/* Rotate.cpp */
#include "Rotate_Vector.hpp"
#include "VectorTraits.hpp"
#include <stdexcept>

template class ImageRotator_V<uint8_t>;
template class ImageRotator_V<uint16_t>;
// template class ImageRotator_V<uint32_t>;
// template class ImageRotator_V<uint64_t>;

template <typename T>
void ImageRotator_V<T>::rotate(Image<T> &image, RotationDirection_V direction)
{
    if (image.metadata.width == 0 || image.metadata.height == 0)
    {
        throw RotationError_V("Invalid image dimensions for rotation.");
    }

    if (image.pixelMatrix.empty())
    {
        throw RotationError_V("Pixel matrix is empty, cannot rotate image.");
    }

    switch (direction)
    {
    case RotationDirection_V::CW_90:
        __riscv_rotate90CW(image.pixelMatrix);
        break;
    case RotationDirection_V::CCW_90:
        __riscv_rotate90CCW(image.pixelMatrix);
        break;
    case RotationDirection_V::ROTATE_180:
        __riscv_rotate180(image.pixelMatrix);
        break;
    default:
        throw RotationError_V("Unsupported rotation direction.");
    }

    image.metadata.width = image.pixelMatrix[0].size();
    image.metadata.height = image.pixelMatrix.size();
}

template <typename T>
void ImageRotator_V<T>::__riscv_rotate90CW(std::vector<std::vector<T>>& matrix) {
    size_t height = matrix.size();
    size_t width = matrix[0].size();
    std::vector<std::vector<T>> rotated(width, std::vector<T>(height));

    // For 90° CW rotation: new[x][y] = old[height-1-y][x]
    // While the transposition itself can't be vectorized efficiently,
    // we can vectorize the row operations and memory copying
    
    for (size_t new_row = 0; new_row < width; ++new_row) {
        // Vectorized approach: collect the column data first, then store efficiently
        size_t old_col = new_row;
        
        // Create a temporary buffer for the entire row
        std::vector<T> temp_row(height);
        
        // Collect column data (this part is inherently scalar due to stride access)
        for (size_t new_col = 0; new_col < height; ++new_col) {
            size_t old_row = height - 1 - new_col;
            temp_row[new_col] = matrix[old_row][old_col];
        }
        
        // Vectorized copy of the entire row to destination
        T* dest_row = rotated[new_row].data();
        const T* src_row = temp_row.data();
        
        size_t j = 0;
        while (j < height) {
            size_t remaining = height - j;
            size_t vl = VectorTraits<T>::vsetvl(remaining);
            
            auto vec_data = VectorTraits<T>::vle(&src_row[j], vl);
            VectorTraits<T>::vse(&dest_row[j], vec_data, vl);
            
            j += vl;
        }
    }
    
    matrix = std::move(rotated);
}

template <typename T>
void ImageRotator_V<T>::__riscv_rotate90CCW(std::vector<std::vector<T>>& matrix) {
    size_t height = matrix.size();
    size_t width = matrix[0].size();
    std::vector<std::vector<T>> rotated(width, std::vector<T>(height));

    // For 90° CCW rotation: new[x][y] = old[y][width-1-x]
    // Use the same vectorized approach for row copying
    
    for (size_t new_row = 0; new_row < width; ++new_row) {
        // Vectorized approach: collect the column data first, then store efficiently
        size_t old_col = width - 1 - new_row;
        
        // Create a temporary buffer for the entire row
        std::vector<T> temp_row(height);
        
        // Collect column data (this part is inherently scalar due to stride access)
        for (size_t new_col = 0; new_col < height; ++new_col) {
            size_t old_row = new_col;
            temp_row[new_col] = matrix[old_row][old_col];
        }
        
        // Vectorized copy of the entire row to destination
        T* dest_row = rotated[new_row].data();
        const T* src_row = temp_row.data();
        
        size_t j = 0;
        while (j < height) {
            size_t remaining = height - j;
            size_t vl = VectorTraits<T>::vsetvl(remaining);
            
            auto vec_data = VectorTraits<T>::vle(&src_row[j], vl);
            VectorTraits<T>::vse(&dest_row[j], vec_data, vl);
            
            j += vl;
        }
    }
    
    matrix = std::move(rotated);
}

template <typename T>
void ImageRotator_V<T>::__riscv_rotate180(std::vector<std::vector<T>> &matrix)
{
    size_t rows = matrix.size();
    size_t cols = matrix[0].size();
    
    // 180° rotation can be efficiently vectorized as it's essentially
    // flipping both horizontally and vertically. We can process rows in chunks.
    
    // Process only half the rows to avoid double-swapping
    for (size_t i = 0; i < (rows + 1) / 2; ++i) {
        T* top_row = matrix[i].data();
        T* bottom_row = matrix[rows - 1 - i].data();
        
        if (i == rows - 1 - i) {
            // Middle row case (odd number of rows) - just reverse this row
            size_t j = 0;
            while (j < cols / 2) {
                size_t remaining = cols / 2 - j;
                size_t vl = VectorTraits<T>::vsetvl(remaining);
                
                // Ensure we don't cross the middle
                vl = std::min(vl, cols / 2 - j);
                
                // Load left side
                auto left_vec = VectorTraits<T>::vle(&top_row[j], vl);
                // Load right side (mirrored)
                auto right_vec = VectorTraits<T>::vle(&top_row[cols - j - vl], vl);
                
                // Reverse right vector and store to left
                auto reversed_right = VectorTraits<T>::reverse_vector(right_vec, vl);
                VectorTraits<T>::vse(&top_row[j], reversed_right, vl);
                
                // Reverse left vector and store to right
                auto reversed_left = VectorTraits<T>::reverse_vector(left_vec, vl);
                VectorTraits<T>::vse(&top_row[cols - j - vl], reversed_left, vl);
                
                j += vl;
            }
        } else {
            // Different rows - swap and reverse both
            size_t j = 0;
            while (j < cols) {
                size_t remaining = cols - j;
                size_t vl = VectorTraits<T>::vsetvl(remaining);
                
                // Load both rows
                auto top_vec = VectorTraits<T>::vle(&top_row[j], vl);
                auto bottom_vec = VectorTraits<T>::vle(&bottom_row[j], vl);
                
                // Reverse both vectors
                auto rev_top = VectorTraits<T>::reverse_vector(top_vec, vl);
                auto rev_bottom = VectorTraits<T>::reverse_vector(bottom_vec, vl);
                
                // Store to swapped positions (mirrored within rows)
                VectorTraits<T>::vse(&bottom_row[cols - j - vl], rev_top, vl);
                VectorTraits<T>::vse(&top_row[cols - j - vl], rev_bottom, vl);
                
                j += vl;
            }
        }
    }
}

#endif // ROTATE_CPP