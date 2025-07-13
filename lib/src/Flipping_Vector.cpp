#ifndef FLIPPINGV_CPP
#define FLIPPINGV_CPP

#include "Flipping_Vector.hpp"
#include "VectorTraits.hpp"
#include <vector>
#include <iostream>
#include <riscv_vector.h>
#include <algorithm>

// Explicit template instantiations
template class ImageFlipper_V<uint8_t>;
template class ImageFlipper_V<uint16_t>;
// template class ImageFlipper_V<uint32_t>;
// template class ImageFlipper_V<uint64_t>;

template <typename T>
void ImageFlipper_V<T>::flip(Image<T> &image, FlippingDirection_V direction)
{
    if (image.pixelMatrix.empty())
    {
        throw FlipError_V("Pixel matrix is empty, cannot Flip image.");
    }
    switch (direction)
    {
    case FlippingDirection_V::VERTICAL:
        flipVertical(image.pixelMatrix);
        break;
    case FlippingDirection_V::HORIZONTAL:
        flipHorizontal(image.pixelMatrix);
        break;
    default:
        throw FlipError_V("Unsupported Flip direction.");
    }

    image.metadata.width = image.pixelMatrix[0].size();
    image.metadata.height = image.pixelMatrix.size();
}

template <typename T>
void ImageFlipper_V<T>::flipVertical(vector<vector<T>> &matrix)
{
    size_t height = matrix.size();
    size_t width = matrix[0].size();

    // Process only half the rows; swap row i with row (height - 1 - i)
    for (size_t i = 0; i < height / 2; ++i)
    {
        // Use pointers for direct memory access
        T* rowTop = matrix[i].data();
        T* rowBottom = matrix[height - 1 - i].data();

        size_t j = 0;
        while (j < width)
        {
            size_t remaining = width - j;
            size_t vl = VectorTraits<T>::vsetvl(remaining);

            // Load vector chunks from both rows
            auto vTop = VectorTraits<T>::vle(&rowTop[j], vl);
            auto vBottom = VectorTraits<T>::vle(&rowBottom[j], vl);

            // Store the swapped data back
            VectorTraits<T>::vse(&rowTop[j], vBottom, vl);
            VectorTraits<T>::vse(&rowBottom[j], vTop, vl);

            j += vl;
        }
    }
}

template <typename T>
void ImageFlipper_V<T>::flipHorizontal(vector<vector<T>> &matrix)
{
    size_t height = matrix.size();
    size_t width = matrix[0].size();

    for (size_t i = 0; i < height; ++i)
    {
        T* row = matrix[i].data();
        
        // Most efficient in-place vectorized horizontal flipping
        // Process pairs of chunks from both ends moving toward center
        size_t left = 0;
        size_t right = width;
        
        while (left < right)
        {
            // Calculate maximum vector length we can process
            size_t max_vl = (right - left) / 2;
            if (max_vl == 0) break;
            
            size_t vl = VectorTraits<T>::vsetvl(max_vl);
            
            // Ensure we don't process more than what's available
            if (left + vl >= right - vl) {
                // Fall back to scalar for the remaining elements to avoid overlap
                while (left < right - 1) {
                    right--;
                    std::swap(row[left], row[right]);
                    left++;
                }
                break;
            }
            
            // Load left chunk
            auto left_vec = VectorTraits<T>::vle(&row[left], vl);
            
            // Load right chunk (from the end, moving backward)
            auto right_vec = VectorTraits<T>::vle(&row[right - vl], vl);
            
            // Reverse both chunks to maintain correct order
            auto rev_left = VectorTraits<T>::reverse_vector(left_vec, vl);
            auto rev_right = VectorTraits<T>::reverse_vector(right_vec, vl);
            
            // Store swapped chunks
            VectorTraits<T>::vse(&row[left], rev_right, vl);
            VectorTraits<T>::vse(&row[right - vl], rev_left, vl);
            
            // Move inward
            left += vl;
            right -= vl;
        }
    }
}

#endif // FLIPPINGV_CPP

          
