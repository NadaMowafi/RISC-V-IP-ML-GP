
#ifndef ADDRVV_HPP
#define ADDRVV_HPP

#include <vector>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <riscv_vector.h>
using namespace std;

/**
 * @brief Overflow handling modes for image arithmetic operations
 */
enum class CLIPPING
{
    SATURATION, ///< Clamp values to [0, 255] range when overflow occurs
    WRAP        ///< Allow values to wrap around on overflow (modulo 256)
};

/**
 * @brief Performs element-wise addition of two 8-bit grayscale images using RISC-V Vector Extension
 * 
 * This function adds two square grayscale images pixel by pixel using RISC-V Vector (RVV) 
 * instructions for optimal performance. Images of different sizes are automatically centered
 * in the output buffer, with the output size determined by the larger input image.
 * 
 * @param image1 First input image as 2D vector of uint8_t pixels (must be square)
 * @param image2 Second input image as 2D vector of uint8_t pixels (must be square)  
 * @param clip   Overflow handling mode (SATURATION or WRAP)
 * 
 * @return New image containing the element-wise sum of input images
 * 
 * @throws std::invalid_argument if either image is empty or non-square
 * @throws std::invalid_argument if invalid clipping mode is specified

 * 
 * @note Time Complexity: O(M*N) where M*N is the size of the larger image
 * @note Space Complexity: O(M*N) for output and temporary buffers
 * @note This implementation uses RISC-V Vector Extension intrinsics for vectorized processing
 * 
 * @details
 * Algorithm Overview:
 * 1. Validate input images (non-empty, square dimensions)
 * 2. Calculate output dimensions and centering offsets
 * 3. Create zero-initialized output and temporary buffers
 * 4. Copy and center image1 to output buffer using vectorized loads/stores
 * 5. Copy and center image2 to temporary buffer using vectorized loads/stores  
 * 6. Perform vectorized addition with overflow handling:
 *    - SATURATION: Clamp results to [0, 255] range
 *    - WRAP: Allow arithmetic overflow to wrap around
 * 
 * @example
 * ```cpp
 * vector<vector<uint8_t>> img1 = {{100, 150}, {200, 50}};
 * vector<vector<uint8_t>> img2 = {{50, 100}, {30, 200}};
 * auto result = __riscv_Add(img1, img2, CLIPPING::SATURATION);
 * // result = {{150, 250}, {230, 250}} (255 clamped)
 * ```
 */
vector<vector<uint8_t>> __riscv_Add(const vector<vector<uint8_t>>& image1, const vector<vector<uint8_t>>& image2 , CLIPPING clip);

/**
 * @brief Performs weighted addition of two 8-bit grayscale images using RISC-V Vector Extension
 * 
 * This function performs weighted linear combination of two square grayscale images using the formula:
 * result = alpha * image1 + beta * image2 + gamma
 * 
 * The implementation uses RISC-V Vector (RVV) instructions with 16-bit floating-point precision
 * for accurate weighted calculations. Images of different sizes are automatically centered
 * in the output buffer, with the output size determined by the larger input image.
 * 
 * @param image1 First input image as 2D vector of uint8_t pixels (must be square)
 * @param alpha  Weight coefficient for first image (floating-point multiplier)
 * @param image2 Second input image as 2D vector of uint8_t pixels (must be square)
 * @param beta   Weight coefficient for second image (floating-point multiplier)
 * @param gamma  Scalar value added to weighted sum (brightness offset)
 * @param clip   Overflow handling mode (SATURATION or WRAP)
 * 
 * @return New image containing the weighted combination of input images
 * 
 * @throws std::invalid_argument if either image is empty or non-square
 * @throws std::invalid_argument if invalid clipping mode is specified
 * 
 * @note Time Complexity: O(M*N) where M*N is the size of the larger image
 * @note Space Complexity: O(M*N) for output and temporary buffers
 * @note This implementation uses RISC-V Vector Extension intrinsics with 16-bit float vectors
 * 
 * @details
 * Algorithm Overview:
 * 1. Validate input images (non-empty, square dimensions)
 * 2. Calculate output dimensions and centering offsets
 * 3. For same-size images: Direct weighted addition using vectorized operations
 * 4. For different-size images:
 *    - Create zero-initialized output and temporary buffers
 *    - Copy and center images using vectorized loads/stores
 *    - Perform weighted addition across entire output buffer
 * 5. Floating-point processing pipeline:
 *    - Convert uint8 pixels to 16-bit float vectors
 *    - Apply weight multiplication (alpha, beta)
 *    - Add weighted values and gamma offset
 *    - Convert back to uint8 with overflow handling:
 *      * SATURATION: Automatic clamping to [0, 255] range
 *      * WRAP: Manual bit masking for modulo 256 behavior
 * 
 * @example
 * ```cpp
 * vector<vector<uint8_t>> img1 = {{100, 150}, {200, 50}};
 * vector<vector<uint8_t>> img2 = {{50, 100}, {30, 200}};
 * // Blend images: 70% img1 + 30% img2 + brightness offset
 * auto result = __riscv_AddWeighted(img1, 0.7f, img2, 0.3f, 10.0f, CLIPPING::SATURATION);
 * // result = {{85, 135}, {149, 95}} (calculations: 0.7*100 + 0.3*50 + 10 = 85, etc.)
 * ```
 */
vector<vector<uint8_t>> __riscv_AddWeighted( const vector<vector<uint8_t>>& image1, float alpha, const vector<vector<uint8_t>>& image2, float beta, float gamma, CLIPPING clip);

/**
 * @brief Accumulates an 8-bit grayscale image into an accumulator buffer using RISC-V Vector Extension
 * 
 * This function performs cumulative addition of images into an accumulator buffer using the formula:
 * accumulator = accumulator + image
 * 
 * The implementation uses RISC-V Vector (RVV) instructions for optimal performance. Images of different 
 * sizes are automatically centered in the accumulator buffer, with the buffer size automatically 
 * expanding to accommodate larger images. This function is commonly used for background subtraction,
 * motion detection, and statistical image analysis.
 * 
 * @param accumulator Reference to accumulator buffer (modified in-place, must be square if not empty)
 * @param image       Input image to accumulate as 2D vector of uint8_t pixels (must be square)
 * @param clip        Overflow handling mode (SATURATION or WRAP)
 * 
 * @throws std::invalid_argument if input image is empty or non-square
 * @throws std::invalid_argument if accumulator is non-square (when not empty)
 * @throws std::invalid_argument if invalid clipping mode is specified
 * 
 * @note Time Complexity: O(M*N) where M*N is the size of the larger image
 * @note Space Complexity: O(1) additional space (modifies accumulator in-place)
 * @note This implementation uses RISC-V Vector Extension intrinsics for vectorized processing
 * @note If accumulator is empty, it will be initialized with a copy of the input image
 * 
 * @details
 * Algorithm Overview:
 * 1. Validate input image (non-empty, square dimensions)
 * 2. Handle empty accumulator case (direct copy)
 * 3. Validate accumulator (square dimensions)
 * 4. Calculate output dimensions and centering offsets
 * 5. Resize accumulator if needed and center existing data using vectorized operations
 * 6. Perform vectorized accumulation with overflow handling:
 * 7. For each pixel position:
 *    - SATURATION: Clamp results to [0, 255] range using overflow detection
 *    - WRAP: Allow arithmetic overflow to wrap around (modulo 256)
 * 
 * @example
 * ```cpp
 * vector<vector<uint8_t>> accumulator; // Empty initially
 * vector<vector<uint8_t>> img1 = {{100, 150}, {200, 50}};
 * vector<vector<uint8_t>> img2 = {{50, 100}, {30, 200}};
 * 
 * __riscv_Accumulate(accumulator, img1, CLIPPING::SATURATION);
 * // accumulator = {{100, 150}, {200, 50}} (first image copied)
 * 
 * __riscv_Accumulate(accumulator, img2, CLIPPING::SATURATION);
 * // accumulator = {{150, 250}, {230, 250}} (accumulated with saturation)
 * ```
 */
void __riscv_Accumulate (vector<vector<uint8_t>>& accumulator, const vector<vector<uint8_t>>& image, CLIPPING clip);

/**
 * @brief Accumulates squared values of an 8-bit grayscale image into an accumulator buffer using RISC-V Vector Extension
 * 
 * This function performs cumulative addition of squared pixel values into an accumulator buffer using the formula:
 * accumulator = accumulator² + image²
 * 
 * The implementation uses RISC-V Vector (RVV) instructions with 16-bit arithmetic for accurate squared value
 * calculations before conversion back to 8-bit. Images of different sizes are automatically centered in the 
 * accumulator buffer, with the buffer size automatically expanding to accommodate larger images. This function
 * is commonly used for variance calculation, background modeling with higher-order statistics, and enhanced
 * motion detection algorithms requiring squared difference computations.
 * 
 * @param accumulator Reference to accumulator buffer (modified in-place, must be square if not empty)
 * @param image       Input image to accumulate as 2D vector of uint8_t pixels (must be square)
 * @param clip        Overflow handling mode (SATURATION or WRAP)
 * 
 * @throws std::invalid_argument if input image is empty or non-square
 * @throws std::invalid_argument if accumulator is non-square (when not empty)
 * @throws std::invalid_argument if invalid clipping mode is specified
 * 
 * @note Time Complexity: O(M*N) where M*N is the size of the larger image
 * @note Space Complexity: O(1) additional space (modifies accumulator in-place)
 * @note This implementation uses RISC-V Vector Extension intrinsics with 16-bit intermediate arithmetic
 * @note If accumulator is empty, it will be initialized with squared values of the input image
 * @note Maximum input pixel value before saturation: √255 ≈ 15.97, so values > 15 may saturate
 * 
 * @details
 * Algorithm Overview:
 * 1. Validate input image (non-empty, square dimensions)
 * 2. Handle empty accumulator case (square input image and store)
 * 3. Validate accumulator (square dimensions)
 * 4. Calculate output dimensions and centering offsets
 * 5. Resize accumulator if needed and center existing data using vectorized operations
 * 6. Perform vectorized squared accumulation with 16-bit intermediate precision:
 *    - Load 8-bit accumulator and image vectors
 *    - Compute squares using widening multiplication (8-bit → 16-bit)
 *    - Add squared values in 16-bit precision
 *    - Convert back to 8-bit with overflow handling:
 *      * SATURATION: Clamp results to [0, 255] range using overflow detection
 *      * WRAP: Allow arithmetic overflow to wrap around (modulo 256)
 * 
 * @example
 * ```cpp
 * vector<vector<uint8_t>> accumulator; // Empty initially
 * vector<vector<uint8_t>> img1 = {{10, 15}, {12, 8}};
 * vector<vector<uint8_t>> img2 = {{5, 10}, {7, 12}};
 * 
 * __riscv_AccumulateSquared(accumulator, img1, CLIPPING::SATURATION);
 * // accumulator = {{100, 225}, {144, 64}} (10²=100, 15²=225, etc.)
 * 
 * __riscv_AccumulateSquared(accumulator, img2, CLIPPING::SATURATION);
 * // accumulator = {{125, 255}, {193, 208}} (100+25=125, 225+100=255(saturated), etc.)
 * ```
 */
void __riscv_AccumulateSquared( vector<vector<uint8_t>>& accumulator, const vector<vector<uint8_t>>& image, CLIPPING clip);

#endif // ADDRVV_HPP