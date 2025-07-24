#include "AddRVV.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>

/**
 * @brief Maximum value for 8-bit unsigned integer pixel values
 */
constexpr uint8_t MAX_PIXEL_VALUE = 255;

/**
 * @brief Performs element-wise addition of two 8-bit grayscale images using RISC-V Vector Extension
 * 
 * This function adds corresponding pixels from two grayscale images using RISC-V Vector (RVV) 
 * intrinsics. Images of different sizes are automatically centered in the output buffer.
 */
vector<vector<uint8_t>> __riscv_Add(const vector<vector<uint8_t>>& image1, const vector<vector<uint8_t>>& image2, CLIPPING clip)
{
    // =====================================================================
    // PHASE 1: INPUT VALIDATION
    // =====================================================================
    
    // Validate that input images are not empty
    if (image1.empty() || image1[0].empty() || image2.empty() || image2[0].empty()) {
        throw invalid_argument("Input validation failed: One or both images are empty");
    }
    // Validate that input images are square
    if (image1.size() != image1[0].size() || image2.size() != image2[0].size()) {
        throw invalid_argument("Input validation failed: Images must be square (rows == columns)");
    }

    // Extract dimensions for processing
    const size_t rows1 = image1.size();
    const size_t cols1 = image1[0].size();
    const size_t rows2 = image2.size();    
    const size_t cols2 = image2[0].size();

    // =====================================================================
    // PHASE 2: DIMENSION CALCULATION AND CENTERING SETUP
    // =====================================================================

    // Determine if images have the same dimensions
    const bool sameDimensions = (rows1 == rows2 && cols1 == cols2);

    // Calculate output dimensions (accommodate larger image)
    const size_t outputRows = max(rows1, rows2);
    const size_t outputCols = max(cols1, cols2);

    // Calculate centering offsets for image alignment
    // Smaller images are centered within the larger output buffer
    const size_t rowOffset1 = (outputRows - rows1) / 2;
    const size_t colOffset1 = (outputCols - cols1) / 2;
    const size_t rowOffset2 = (outputRows - rows2) / 2;
    const size_t colOffset2 = (outputCols - cols2) / 2;

    // =====================================================================
    // PHASE 3: MEMORY ALLOCATION
    // =====================================================================
    // Declare output image and initialize it with zeros
    vector<vector<uint8_t>> outputImg(outputRows, vector<uint8_t>(outputCols, 0));
    // =====================================================================
    // PHASE 4: SAME-SIZE ADDITION PROCESSING
    // =====================================================================

     if (sameDimensions) {
        // Direct addition without centering operations
           // Process based on clipping mode
        if (clip == CLIPPING::WRAP) {
            // WRAP MODE: Allow arithmetic overflow to wrap around
            /*Process of adding two images using wrap mode:
            1) set the vector length (vl) based on the remaining columns to process.
            2) Load the pixel values from both images into vector registers using __riscv_vle8_v_u8m1.
            3) Add the two vectors using __riscv_vadd_vv_u8m1.
             **If it exceeded the maximum  value of add function then it will wrap.
            4) Store the result back to the output image using __riscv_vse8_v_u8m1.
            */
            for (size_t i = 0; i < outputRows; i++) {
                size_t j = 0;
                while (j < outputCols) {
                    size_t vl = __riscv_vsetvl_e8m1(cols1 - j);
                    vuint8m1_t vec1 = __riscv_vle8_v_u8m1(&image1[i][j], vl);
                    vuint8m1_t vec2 = __riscv_vle8_v_u8m1(&image2[i][j], vl);
                    vuint8m1_t result = __riscv_vadd_vv_u8m1(vec1, vec2, vl);
                    __riscv_vse8_v_u8m1(&outputImg[i][j], result, vl);
                    j += vl;
                }
            }
        } else if (clip == CLIPPING::SATURATION) {
            // SATURATION MODE: Clamp results to valid range
            /* Process of adding two images using saturation mode:
            1) Set the vector length (vl) based on the remaining columns to process.
            2) Load the pixel values from both images into vector registers using __riscv_vle8_v_u8m1.
            3) Add the two vectors using __riscv_vadd_vv_u8m1.
            4) Check for overflow using __riscv_vmsltu_vv_u8m1_b8 to determine if the sum exceeds the maximum pixel value.
            ** Comparing Sum with value of vec1 to check if it exceeds the maximum value.
            5) If overflow occurs, merge the saturated value (255) with the sum using __riscv_vmerge_vxm_u8m1.
            6) Store the result back to the output image using __riscv_vse8_v_u8m1.
            */
            for (size_t i = 0; i < outputRows; i++) {
                size_t j = 0;     
                while (j < outputCols) {
                    size_t vl = __riscv_vsetvl_e8m1(cols1 - j);
                    vuint8m1_t vec1 = __riscv_vle8_v_u8m1(&image1[i][j], vl);
                    vuint8m1_t vec2 = __riscv_vle8_v_u8m1(&image2[i][j], vl);
                    vuint8m1_t sum = __riscv_vadd_vv_u8m1(vec1, vec2, vl);
                    vbool8_t overflow = __riscv_vmsltu_vv_u8m1_b8(sum, vec1, vl);
                    vuint8m1_t saturated = __riscv_vmerge_vxm_u8m1(sum, 255, overflow, vl);
                    __riscv_vse8_v_u8m1(&outputImg[i][j], saturated, vl);
                    j += vl;
                }
            }
        } else {
            throw invalid_argument("Invalid clipping mode specified");
        }
        
        return outputImg;
    }

    // =====================================================================
    // PHASE 5: DIFFERENT-SIZE ADDITION PROCESSING
    // =====================================================================
    
    // Copy image1 to outputImg
    /* Process of copying image1 to outputImg:
    1) Set the vector length (vl) based on the remaining columns to process.
    2) Load the pixel values from image1 into vector registers using __riscv_vle8_v_u8m1.
    3) Store the vectorized pixel values into the output image at the appropriate offset using __riscv_vse8_v_u8m1.
    */
    for (size_t i = 0; i < rows1; i++) {
        size_t j = 0;
        while (j < cols1) {
            size_t vl = __riscv_vsetvl_e8m1(cols1 - j);
            vuint8m1_t vsrc = __riscv_vle8_v_u8m1(&image1[i][j], vl);
            __riscv_vse8_v_u8m1(&outputImg[rowOffset1 + i][colOffset1 + j], vsrc, vl);
            j += vl;
        }
    }

    // Declare a temporary image buffer to hold image2 and initialize it with zeros
    vector<vector<uint8_t>> tempImg(outputRows, vector<uint8_t>(outputCols, 0));

    // Copy image2 to tempImg
    /* Process of copying image2 to tempImg:
    1) Set the vector length (vl) based on the remaining columns to process.
    2) Load the pixel values from image2 into vector registers using __riscv_vle8_v_u8m1.
    3) Store the vectorized pixel values into the temporary image at the appropriate offset using __riscv_vse8_v_u8m1.
    */
    for (size_t i = 0; i < rows2; i++) {
        size_t j = 0;
        while (j < cols2) {
            size_t vl = __riscv_vsetvl_e8m1(cols2 - j);
            vuint8m1_t vsrc = __riscv_vle8_v_u8m1(&image2[i][j], vl);
            __riscv_vse8_v_u8m1(&tempImg[rowOffset2 + i][colOffset2 + j], vsrc, vl);
            j += vl;
        }
    }
    // =====================================================================
     if (clip == CLIPPING::WRAP) {
            // WRAP MODE: Allow arithmetic overflow to wrap around
            /*Process of adding two images using wrap mode:
            1) set the vector length (vl) based on the remaining columns to process.
            2) Load the pixel values from both images into vector registers using __riscv_vle8_v_u8m1.
            3) Add the two vectors using __riscv_vadd_vv_u8m1.
             **If it exceeded the maximum  value of add function then it will wrap.
            4) Store the result back to the output image using __riscv_vse8_v_u8m1.
            */
            for (size_t i = 0; i < outputRows; i++) {
                size_t j = 0;
                while (j < outputCols) {
                    size_t vl = __riscv_vsetvl_e8m1(cols1 - j);
                    vuint8m1_t vec1 = __riscv_vle8_v_u8m1(&tempImg[i][j], vl);
                    vuint8m1_t vec2 = __riscv_vle8_v_u8m1(&outputImg[i][j], vl);
                    vuint8m1_t result = __riscv_vadd_vv_u8m1(vec1, vec2, vl);
                    __riscv_vse8_v_u8m1(&outputImg[i][j], result, vl);
                    j += vl;
                }
            }
        } else if (clip == CLIPPING::SATURATION) {
            // SATURATION MODE: Clamp results to valid range
            /* Process of adding two images using saturation mode:
            1) Set the vector length (vl) based on the remaining columns to process.
            2) Load the pixel values from both images into vector registers using __riscv_vle8_v_u8m1.
            3) Add the two vectors using __riscv_vadd_vv_u8m1.
            4) Check for overflow using __riscv_vmsltu_vv_u8m1_b8 to determine if the sum exceeds the maximum pixel value.
            ** Comparing Sum with value of vec1 to check if it exceeds the maximum value.
            5) If overflow occurs, merge the saturated value (255) with the sum using __riscv_vmerge_vxm_u8m1.
            6) Store the result back to the output image using __riscv_vse8_v_u8m1.
            */
            for (size_t i = 0; i < outputRows; i++) {
                size_t j = 0;     
                while (j < outputCols) {
                    size_t vl = __riscv_vsetvl_e8m1(cols1 - j);
                    vuint8m1_t vec1 = __riscv_vle8_v_u8m1(&tempImg[i][j], vl);
                    vuint8m1_t vec2 = __riscv_vle8_v_u8m1(&outputImg[i][j], vl);
                    vuint8m1_t sum = __riscv_vadd_vv_u8m1(vec1, vec2, vl);
                    vbool8_t overflow = __riscv_vmsltu_vv_u8m1_b8(sum, vec1, vl);
                    vuint8m1_t saturated = __riscv_vmerge_vxm_u8m1(sum, 255, overflow, vl);
                    __riscv_vse8_v_u8m1(&outputImg[i][j], saturated, vl);
                    j += vl;
                }
            }
        } else {
            throw invalid_argument("Invalid clipping mode specified");
        }
        
        return outputImg;

    }

//==================AddWeighted Function==================

/**
 * @brief Performs weighted addition of two 8-bit grayscale images using RISC-V Vector Extension
 * 
 * This function performs weighted addition using the formula: result = alpha*image1 + beta*image2 + gamma
 * Images of different sizes are automatically centered in the output buffer.
 */
vector<vector<uint8_t>> __riscv_AddWeighted(const vector<vector<uint8_t>>& image1, float alpha, const vector<vector<uint8_t>>& image2, float beta, float gamma, CLIPPING clip)
{
    // =====================================================================
    // PHASE 1: INPUT VALIDATION
    // =====================================================================
    
    // Validate that input images are not empty
    if (image1.empty() || image1[0].empty() || image2.empty() || image2[0].empty()) {
        throw invalid_argument("Input validation failed: One or both images are empty");
    }
    
    // Validate that input images are square (required by function specification)
    if (image1.size() != image1[0].size() || image2.size() != image2[0].size()) {
        throw invalid_argument("Input validation failed: Images must be square (rows == columns)");
    }

    // Extract dimensions for processing
    const size_t rows1 = image1.size();
    const size_t cols1 = image1[0].size();
    const size_t rows2 = image2.size();
    const size_t cols2 = image2[0].size();

    // =====================================================================
    // PHASE 2: DIMENSION CALCULATION AND CENTERING SETUP
    // =====================================================================

    // Determine if images have the same dimensions
    const bool sameDimensions = (rows1 == rows2 && cols1 == cols2);

    // Calculate output dimensions (accommodate larger image)
    const size_t outputRows = max(rows1, rows2);
    const size_t outputCols = max(cols1, cols2);

    // Calculate centering offsets for image alignment
    // Smaller images are centered within the larger output buffer
    const size_t rowOffset1 = (outputRows - rows1) / 2;
    const size_t colOffset1 = (outputCols - cols1) / 2;
    const size_t rowOffset2 = (outputRows - rows2) / 2;
    const size_t colOffset2 = (outputCols - cols2) / 2;

    // =====================================================================
    // PHASE 3: MEMORY ALLOCATION
    // =====================================================================
    
    // Declare output image buffer
    vector<vector<uint8_t>> outputImg(outputRows, vector<uint8_t>(outputCols, 0));

    // =====================================================================
    // PHASE 4: SAME-SIZE IMAGE PROCESSING
    // =====================================================================
    
    if (sameDimensions) {
        // Direct weighted addition without centering operations
        // Process based on clipping mode
        if (clip == CLIPPING::WRAP) {
            // WRAP MODE: Allow arithmetic overflow to wrap around
            /* Process of weighted addition using wrap mode:
            1) Set the vector length (vl) based on the remaining columns to process.
            2) Load pixel values from both images into vector registers using __riscv_vle8_v_u8m1.
            3) Convert to floating-point format using __riscv_vfwcvt_f_xu_v_f16m2.
            4) Multiply by weights (alpha and beta) using __riscv_vfmul_vf_f16m2.
            5) Add weighted values and gamma using __riscv_vfadd_vv_f16m2 and __riscv_vfadd_vf_f16m2_rm.
            6) Convert back to integer and apply wrap clipping using bit masking.
            7) Store result back to output image using __riscv_vse8_v_u8m1.
            */
            for (size_t i = 0; i < rows1; i++) {
                outputImg[i].resize(cols1);
                size_t j = 0;
                
                while (j < cols1) {
                    size_t vl = __riscv_vsetvl_e8m1(cols1 - j);
                    
                    // Load pixel values from both images
                    vuint8m1_t v1_u8 = __riscv_vle8_v_u8m1(&image1[i][j], vl);
                    vuint8m1_t v2_u8 = __riscv_vle8_v_u8m1(&image2[i][j], vl);
                    
                    // Convert to floating-point for weighted calculations
                    vfloat16m2_t v1_f16 = __riscv_vfwcvt_f_xu_v_f16m2(v1_u8, vl);
                    vfloat16m2_t v2_f16 = __riscv_vfwcvt_f_xu_v_f16m2(v2_u8, vl);
                    
                    // Apply weights (alpha and beta)
                    vfloat16m2_t v1_f16_alpha = __riscv_vfmul_vf_f16m2(v1_f16, (_Float16)alpha, vl);
                    vfloat16m2_t v2_f16_beta = __riscv_vfmul_vf_f16m2(v2_f16, (_Float16)beta, vl);
                    
                    // Combine weighted values and add gamma
                    vfloat16m2_t sum = __riscv_vfadd_vv_f16m2(v1_f16_alpha, v2_f16_beta, vl);
                    vfloat16m2_t sum_gamma = __riscv_vfadd_vf_f16m2_rm(sum, (_Float16)gamma, 0, vl);
                    
                    // Convert back to integer with wrap clipping
                    vuint16m2_t temp_u16 = __riscv_vfcvt_xu_f_v_u16m2(sum_gamma, vl);
                    vuint16m2_t temp_u16_wrapped = __riscv_vand_vx_u16m2(temp_u16, 0xFF, vl);
                    vuint8m1_t result = __riscv_vnclipu_wx_u8m1(temp_u16_wrapped, 0, 0, vl);
                    __riscv_vse8_v_u8m1(&outputImg[i][j], result, vl);
                    
                    j += vl;
                }
            }
        } else if (clip == CLIPPING::SATURATION) {
            // SATURATION MODE: Clamp results to valid range [0, 255]
            /* Process of weighted addition using saturation mode:
            1) Set the vector length (vl) based on the remaining columns to process.
            2) Load pixel values from both images into vector registers using __riscv_vle8_v_u8m1.
            3) Convert to floating-point format using __riscv_vfwcvt_f_xu_v_f16m2.
            4) Multiply by weights (alpha and beta) using __riscv_vfmul_vf_f16m2.
            5) Add weighted values and gamma using __riscv_vfadd_vv_f16m2 and __riscv_vfadd_vf_f16m2_rm.
            6) Convert back to integer with automatic saturation using __riscv_vfncvt_xu_f_w_u8m1.
            7) Store result back to output image using __riscv_vse8_v_u8m1.
            */
            for (size_t i = 0; i < rows1; i++) {
                outputImg[i].resize(cols1);
                size_t j = 0;
                
                while (j < cols1) {
                    size_t vl = __riscv_vsetvl_e8m1(cols1 - j);
                    
                    // Load pixel values from both images
                    vuint8m1_t v1_u8 = __riscv_vle8_v_u8m1(&image1[i][j], vl);
                    vuint8m1_t v2_u8 = __riscv_vle8_v_u8m1(&image2[i][j], vl);
                    
                    // Convert to floating-point for weighted calculations
                    vfloat16m2_t v1_f16 = __riscv_vfwcvt_f_xu_v_f16m2(v1_u8, vl);
                    vfloat16m2_t v2_f16 = __riscv_vfwcvt_f_xu_v_f16m2(v2_u8, vl);
                    
                    // Apply weights (alpha and beta)
                    vfloat16m2_t v1_f16_alpha = __riscv_vfmul_vf_f16m2(v1_f16, (_Float16)alpha, vl);
                    vfloat16m2_t v2_f16_beta = __riscv_vfmul_vf_f16m2(v2_f16, (_Float16)beta, vl);
                    
                    // Combine weighted values and add gamma
                    vfloat16m2_t sum = __riscv_vfadd_vv_f16m2(v1_f16_alpha, v2_f16_beta, vl);
                    vfloat16m2_t sum_gamma = __riscv_vfadd_vf_f16m2_rm(sum, (_Float16)gamma, 0, vl);
                    
                    // Convert back to integer with automatic saturation
                    vuint8m1_t result = __riscv_vfncvt_xu_f_w_u8m1(sum_gamma, vl);
                    __riscv_vse8_v_u8m1(&outputImg[i][j], result, vl);
                    
                    j += vl;
                }
            }
        } else {
            throw invalid_argument("Invalid clipping mode specified");
        }
        
        return outputImg;
    }

    // =====================================================================
    // PHASE 5: MEMORY ALLOCATION FOR DIFFERENT-SIZE IMAGES
    // =====================================================================
    
    // Allocate output buffer with proper dimensions
    
    for (size_t i = 0; i < outputRows; ++i) {
        outputImg[i].resize(outputCols, 0);
    }
    
    // Allocate temporary buffer for second image
    vector<vector<uint8_t>> tempImg(outputRows, vector<uint8_t>(outputCols, 0));

    // =====================================================================
    // PHASE 6: IMAGE COPYING AND CENTERING
    // =====================================================================
    
    // Copy and center image1 to outputImg using RISC-V vector operations
    /* Process of copying image1 to outputImg:
    1) Set the vector length (vl) based on the remaining columns to process.
    2) Load pixel values from image1 into vector registers using __riscv_vle8_v_u8m1.
    3) Store vectorized pixel values into output image at appropriate offset using __riscv_vse8_v_u8m1.
    */
    for (size_t i = 0; i < rows1; i++) {
        size_t j = 0;
        while (j < cols1) {
            size_t vl = __riscv_vsetvl_e8m1(cols1 - j);
            vuint8m1_t vsrc = __riscv_vle8_v_u8m1(&image1[i][j], vl);
            __riscv_vse8_v_u8m1(&outputImg[rowOffset1 + i][colOffset1 + j], vsrc, vl);
            j += vl;
        }
    }
    // Copy and center image2 to tempImg using RISC-V vector operations
    /* Process of copying image2 to tempImg:
    1) Set the vector length (vl) based on the remaining columns to process.
    2) Load pixel values from image2 into vector registers using __riscv_vle8_v_u8m1.
    3) Store vectorized pixel values into temporary image at appropriate offset using __riscv_vse8_v_u8m1.
    */
    for (size_t i = 0; i < rows2; i++) {
        size_t j = 0;
        while (j < cols2) {
            size_t vl = __riscv_vsetvl_e8m1(cols2 - j);
            vuint8m1_t vsrc = __riscv_vle8_v_u8m1(&image2[i][j], vl);
            __riscv_vse8_v_u8m1(&tempImg[rowOffset2 + i][colOffset2 + j], vsrc, vl);
            j += vl;
        }
    }
    // =====================================================================
    // PHASE 7: WEIGHTED ADDITION WITH OVERFLOW HANDLING
    // =====================================================================
    
    // Process weighted addition based on clipping mode
    if (clip == CLIPPING::WRAP) {
        // WRAP MODE: Allow arithmetic overflow to wrap around
        /* Process of weighted addition using wrap mode:
        1) Set the vector length (vl) based on the remaining columns to process.
        2) Load pixel values from both buffers into vector registers using __riscv_vle8_v_u8m1.
        3) Convert to floating-point format using __riscv_vfwcvt_f_xu_v_f16m2.
        4) Multiply by weights (alpha and beta) using __riscv_vfmul_vf_f16m2.
        5) Add weighted values and gamma using __riscv_vfadd_vv_f16m2 and __riscv_vfadd_vf_f16m2_rm.
        6) Convert back to integer and apply wrap clipping using bit masking.
        7) Store result back to output image using __riscv_vse8_v_u8m1.
        */
        for (size_t i = 0; i < outputRows; i++) {
            size_t j = 0;
            while (j < outputCols) {
                size_t vl = __riscv_vsetvl_e8m1(outputCols - j);
                
                // Load pixel values from both centered images
                vuint8m1_t v1_u8 = __riscv_vle8_v_u8m1(&outputImg[i][j], vl);
                vuint8m1_t v2_u8 = __riscv_vle8_v_u8m1(&tempImg[i][j], vl);
                
                // Convert to floating-point for weighted calculations
                vfloat16m2_t v1_f16 = __riscv_vfwcvt_f_xu_v_f16m2(v1_u8, vl);
                vfloat16m2_t v2_f16 = __riscv_vfwcvt_f_xu_v_f16m2(v2_u8, vl);
                
                // Apply weights (alpha and beta)
                vfloat16m2_t v1_f16_alpha = __riscv_vfmul_vf_f16m2(v1_f16, (_Float16)alpha, vl);
                vfloat16m2_t v2_f16_beta = __riscv_vfmul_vf_f16m2(v2_f16, (_Float16)beta, vl);
                
                // Combine weighted values and add gamma
                vfloat16m2_t sum = __riscv_vfadd_vv_f16m2(v1_f16_alpha, v2_f16_beta, vl);
                vfloat16m2_t sum_gamma = __riscv_vfadd_vf_f16m2_rm(sum, (_Float16)gamma, 0, vl);
                
                // Convert back to integer with wrap clipping
                vuint16m2_t temp_u16 = __riscv_vfcvt_xu_f_v_u16m2(sum_gamma, vl);
                vuint16m2_t temp_u16_wrapped = __riscv_vand_vx_u16m2(temp_u16, 0xFF, vl);
                vuint8m1_t result = __riscv_vnclipu_wx_u8m1(temp_u16_wrapped, 0, 0, vl);
                __riscv_vse8_v_u8m1(&outputImg[i][j], result, vl);
                
                j += vl;
            }
        }
    } else if (clip == CLIPPING::SATURATION) {
        // SATURATION MODE: Clamp results to valid range [0, 255]
        /* Process of weighted addition using saturation mode:
        1) Set the vector length (vl) based on the remaining columns to process.
        2) Load pixel values from both buffers into vector registers using __riscv_vle8_v_u8m1.
        3) Convert to floating-point format using __riscv_vfwcvt_f_xu_v_f16m2.
        4) Multiply by weights (alpha and beta) using __riscv_vfmul_vf_f16m2.
        5) Add weighted values and gamma using __riscv_vfadd_vv_f16m2 and __riscv_vfadd_vf_f16m2_rm.
        6) Convert back to integer with automatic saturation using __riscv_vfncvt_xu_f_w_u8m1.
        7) Store result back to output image using __riscv_vse8_v_u8m1.
        */
        for (size_t i = 0; i < outputRows; i++) {
            size_t j = 0;
            while (j < outputCols) {
                size_t vl = __riscv_vsetvl_e8m1(outputCols - j);
                
                // Load pixel values from both centered images
                vuint8m1_t v1_u8 = __riscv_vle8_v_u8m1(&outputImg[i][j], vl);
                vuint8m1_t v2_u8 = __riscv_vle8_v_u8m1(&tempImg[i][j], vl);
                
                // Convert to floating-point for weighted calculations
                vfloat16m2_t v1_f16 = __riscv_vfwcvt_f_xu_v_f16m2(v1_u8, vl);
                vfloat16m2_t v2_f16 = __riscv_vfwcvt_f_xu_v_f16m2(v2_u8, vl);
                
                // Apply weights (alpha and beta)
                vfloat16m2_t v1_f16_alpha = __riscv_vfmul_vf_f16m2(v1_f16, (_Float16)alpha, vl);
                vfloat16m2_t v2_f16_beta = __riscv_vfmul_vf_f16m2(v2_f16, (_Float16)beta, vl);
                
                // Combine weighted values and add gamma
                vfloat16m2_t sum = __riscv_vfadd_vv_f16m2(v1_f16_alpha, v2_f16_beta, vl);
                vfloat16m2_t sum_gamma = __riscv_vfadd_vf_f16m2_rm(sum, (_Float16)gamma, 0, vl);
                
                // Convert back to integer with automatic saturation
                vuint8m1_t result = __riscv_vfncvt_xu_f_w_u8m1(sum_gamma, vl);
                __riscv_vse8_v_u8m1(&outputImg[i][j], result, vl);
                
                j += vl;
            }
        }
    } else {
        throw invalid_argument("Invalid clipping mode specified");
    }
    
    return outputImg;
}

/**
 * @brief Accumulates an 8-bit grayscale image into an accumulator buffer using RISC-V Vector Extension
 * 
 * This function performs cumulative addition of images into an accumulator buffer using the formula:
 * accumulator = accumulator + image. Images of different sizes are automatically centered.
 */
void __riscv_Accumulate(vector<vector<uint8_t>>& accumulator, const vector<vector<uint8_t>>& image, CLIPPING clip)
{
    // =====================================================================
    // PHASE 1: INPUT VALIDATION
    // =====================================================================
    
    // Validate that input image is not empty
    if (image.empty() || image[0].empty()) {
        throw invalid_argument("Input validation failed: Input image is empty");
    }
    
    // Validate that input image is square (required by function specification)
    if (image.size() != image[0].size()) {
        throw invalid_argument("Input validation failed: Input image is not square");
    }
    
    // Handle empty accumulator case - initialize with input image
    if (accumulator.empty()) {
        accumulator = image;
        return;
    }
    
    // Validate that accumulator is square (required by function specification)
    if (accumulator.size() != accumulator[0].size()) {
        throw invalid_argument("Input validation failed: Accumulator is not square");
    }

    // Extract dimensions for processing
    const size_t imgRows = image.size();
    const size_t imgCols = image[0].size();
    const size_t accRows = accumulator.size();
    const size_t accCols = accumulator[0].size();

    // =====================================================================
    // PHASE 2: DIMENSION CALCULATION AND CENTERING SETUP
    // =====================================================================

    // Calculate output dimensions (accommodate larger image/accumulator)
    const size_t outputRows = max(accRows, imgRows);
    const size_t outputCols = max(accCols, imgCols);

    // Calculate centering offsets for image alignment
    // Smaller images/accumulators are centered within the larger output buffer
    const size_t rowOffsetAcc = (outputRows - accRows) / 2;
    const size_t colOffsetAcc = (outputCols - accCols) / 2;
    const size_t rowOffsetImg = (outputRows - imgRows) / 2;
    const size_t colOffsetImg = (outputCols - imgCols) / 2;

    // =====================================================================
    // PHASE 3: ACCUMULATOR RESIZING AND CENTERING
    // =====================================================================

    // Resize accumulator to new output size and center old data if needed
    if (accRows != outputRows || accCols != outputCols) {
        // Create new accumulator buffer with proper dimensions
        vector<vector<uint8_t>> newAcc(outputRows, vector<uint8_t>(outputCols, 0));
        
        // Copy and center existing accumulator data using RISC-V vector operations
        /* Process of copying accumulator to newAcc:
        1) Set the vector length (vl) based on the remaining columns to process.
        2) Load pixel values from accumulator into vector registers using __riscv_vle8_v_u8m1.
        3) Store vectorized pixel values into new accumulator at appropriate offset using __riscv_vse8_v_u8m1.
        */
        for (size_t i = 0; i < accRows; ++i) {
            size_t j = 0;
            while (j < accCols) {
                size_t vl = __riscv_vsetvl_e8m1(accCols - j);
                vuint8m1_t vsrc = __riscv_vle8_v_u8m1(&accumulator[i][j], vl);
                __riscv_vse8_v_u8m1(&newAcc[rowOffsetAcc + i][colOffsetAcc + j], vsrc, vl);
                j += vl;
            }
        }
        accumulator.swap(newAcc);
    }

    // =====================================================================
    // PHASE 4: VECTORIZED ACCUMULATION WITH OVERFLOW HANDLING
    // =====================================================================

    // Accumulate image into accumulator, centered
    /* Process of accumulating image into accumulator:
    1) Set the vector length (vl) based on the remaining columns to process.
    2) Load pixel values from both accumulator and image into vector registers using __riscv_vle8_v_u8m1.
    3) Perform vectorized addition based on clipping mode:
       - WRAP: Simple addition with natural overflow wrapping
       - SATURATION: Addition with overflow detection and clamping to 255
    4) Store result back to accumulator using __riscv_vse8_v_u8m1.
    */
    for (size_t i = 0; i < imgRows; ++i) {
        size_t j = 0;
        while (j < imgCols) {
            size_t vl = __riscv_vsetvl_e8m1(imgCols - j);
            
            // Load pixel values from accumulator and input image
            vuint8m1_t acc_vec = __riscv_vle8_v_u8m1(&accumulator[rowOffsetImg + i][colOffsetImg + j], vl);
            vuint8m1_t img_vec = __riscv_vle8_v_u8m1(&image[i][j], vl);

            if (clip == CLIPPING::WRAP) {
                // WRAP MODE: Allow arithmetic overflow to wrap around
                /* Process of accumulation using wrap mode:
                1) Add accumulator and image vectors using __riscv_vadd_vv_u8m1.
                ** If sum exceeds 255, it will wrap around naturally (modulo 256)
                2) Store result back to accumulator using __riscv_vse8_v_u8m1.
                */
                vuint8m1_t result = __riscv_vadd_vv_u8m1(acc_vec, img_vec, vl);
                __riscv_vse8_v_u8m1(&accumulator[rowOffsetImg + i][colOffsetImg + j], result, vl);
            } else if (clip == CLIPPING::SATURATION) {
                // SATURATION MODE: Clamp results to valid range [0, 255]
                /* Process of accumulation using saturation mode:
                1) Add accumulator and image vectors using __riscv_vadd_vv_u8m1.
                2) Check for overflow using __riscv_vmsltu_vv_u8m1_b8 by comparing sum with accumulator.
                ** If sum < accumulator, overflow occurred (wrapped around)
                3) If overflow occurs, merge the saturated value (255) with the sum using __riscv_vmerge_vxm_u8m1.
                4) Store result back to accumulator using __riscv_vse8_v_u8m1.
                */
                vuint8m1_t sum = __riscv_vadd_vv_u8m1(acc_vec, img_vec, vl);
                vbool8_t overflow = __riscv_vmsltu_vv_u8m1_b8(sum, acc_vec, vl);
                vuint8m1_t saturated = __riscv_vmerge_vxm_u8m1(sum, 255, overflow, vl);
                __riscv_vse8_v_u8m1(&accumulator[rowOffsetImg + i][colOffsetImg + j], saturated, vl);
            } else {
                throw invalid_argument("Invalid clipping mode specified");
            }
            j += vl;
        }
    }
}


 //==================AccumulateSquared Function==================

/**
 * @brief Accumulates squared values of an 8-bit grayscale image into an accumulator buffer using RISC-V Vector Extension
 * 
 * This function performs cumulative addition of squared pixel values with 16-bit intermediate precision
 * for accurate squared value calculations before conversion back to 8-bit format.
 */
void __riscv_AccumulateSquared(vector<vector<uint8_t>>& accumulator, const vector<vector<uint8_t>>& image, CLIPPING clip)
{
    // =====================================================================
    // PHASE 1: INPUT VALIDATION
    // =====================================================================
    
    // Validate that input image is not empty and is properly formatted
    if (image.empty() || image[0].empty()) {
        throw invalid_argument("Input validation failed: Input image is empty");
    }
    
    // Validate that input image is square (required by function specification)
    if (image.size() != image[0].size()) {
        throw invalid_argument("Input validation failed: Input image is not square");
    }
    
    // =====================================================================
    // PHASE 2: HANDLE EMPTY ACCUMULATOR CASE
    // =====================================================================
    
    // If accumulator is empty, initialize it with squared values of the input image
    // This is the base case where we're starting the accumulation process
    if (accumulator.empty()) {
        // Resize accumulator to match input image dimensions
        accumulator.resize(image.size(), vector<uint8_t>(image[0].size()));
        
        // Process each row of the image
        for (size_t i = 0; i < image.size(); ++i) {
            size_t j = 0;
            
            // Vectorized processing of each row using RISC-V Vector Extension
            while (j < image[0].size()) {
                // Set vector length for current iteration (handles remaining elements)
                size_t vl = __riscv_vsetvl_e8m1(image[0].size() - j);
                
                // Load 8-bit pixel values from source image into vector register
                vuint8m1_t img_vec = __riscv_vle8_v_u8m1(&image[i][j], vl);
                
                // Compute squares using widening multiplication (8-bit → 16-bit)
                // This prevents intermediate overflow during squaring operation
                vuint16m2_t img_sqr_16 = __riscv_vwmulu_vv_u16m2(img_vec, img_vec, vl);
                
                vuint8m1_t result;
                
                // Apply overflow handling based on specified clipping mode
                if (clip == CLIPPING::WRAP) {
                    // WRAP mode: Allow overflow to wrap around (modulo 256)
                    // Direct narrowing conversion truncates high bits
                    result = __riscv_vncvt_x_x_w_u8m1(img_sqr_16, vl);
                } else if (clip == CLIPPING::SATURATION) {
                    // SATURATION mode: Clamp values to [0, 255] range
                    // Detect overflow condition (values > 255)
                    vbool8_t overflow = __riscv_vmsgtu_vx_u16m2_b8(img_sqr_16, 255, vl);
                    
                    // Merge: keep original values where no overflow, set 255 where overflow
                    vuint16m2_t saturated = __riscv_vmerge_vxm_u16m2(img_sqr_16, 255, overflow, vl);
                    
                    // Convert saturated 16-bit values back to 8-bit
                    result = __riscv_vncvt_x_x_w_u8m1(saturated, vl);
                }
                
                // Store vectorized results to accumulator buffer
                __riscv_vse8_v_u8m1(&accumulator[i][j], result, vl);
                
                // Advance to next vector chunk
                j += vl;
            }
        }
        return; // Early return for empty accumulator case
    }
    
    // =====================================================================
    // PHASE 3: VALIDATE EXISTING ACCUMULATOR
    // =====================================================================
    
    // Validate that existing accumulator is square
    if (accumulator.size() != accumulator[0].size()) {
        throw invalid_argument("Accumulator validation failed: Accumulator is not square");
    }
    
    // =====================================================================
    // PHASE 4: CALCULATE DIMENSIONS AND OFFSETS
    // =====================================================================
    
    // Extract dimensions of both image and accumulator
    size_t imgRows = image.size();
    size_t imgCols = image[0].size();
    size_t accRows = accumulator.size();
    size_t accCols = accumulator[0].size();
    
    // Calculate output dimensions (maximum of both images)
    size_t outputRows = max(accRows, imgRows);
    size_t outputCols = max(accCols, imgCols);
    
    // Calculate centering offsets for both accumulator and image
    // This ensures both images are centered in the final output buffer
    size_t rowOffsetAcc = (outputRows - accRows) / 2;
    size_t colOffsetAcc = (outputCols - accCols) / 2;
    size_t rowOffsetImg = (outputRows - imgRows) / 2;
    size_t colOffsetImg = (outputCols - imgCols) / 2;
    
    // =====================================================================
    // PHASE 5: RESIZE AND CENTER ACCUMULATOR IF NEEDED
    // =====================================================================
    
    // Resize accumulator to new output size and center old data if dimensions differ
    if (accRows != outputRows || accCols != outputCols) {
        // Create new accumulator buffer with expanded dimensions (zero-initialized)
        vector<vector<uint8_t>> newAcc(outputRows, vector<uint8_t>(outputCols, 0));
        
        // Copy existing accumulator data to centered position in new buffer
        for (size_t i = 0; i < accRows; ++i) {
            size_t j = 0;
            
            // Vectorized copying of existing accumulator data
            while (j < accCols) {
                // Set vector length for current row segment
                size_t vl = __riscv_vsetvl_e8m1(accCols - j);
                
                // Load data from old accumulator position
                vuint8m1_t vsrc = __riscv_vle8_v_u8m1(&accumulator[i][j], vl);
                
                // Store to new centered position in expanded buffer
                __riscv_vse8_v_u8m1(&newAcc[rowOffsetAcc + i][colOffsetAcc + j], vsrc, vl);
                
                // Advance to next vector chunk
                j += vl;
            }
        }
        
        // Replace old accumulator with new expanded and centered version
        accumulator.swap(newAcc);
    }
    
    // =====================================================================
    // PHASE 6: PERFORM VECTORIZED SQUARED ACCUMULATION
    // =====================================================================
    
    // Accumulate squared image values into centered accumulator position
    for (size_t i = 0; i < imgRows; ++i) {
        size_t j = 0;
        
        // Process each row with vectorized RISC-V operations
        while (j < imgCols) {
            // Set vector length for current iteration
            size_t vl = __riscv_vsetvl_e8m1(imgCols - j);
            
            // Load current accumulator values from centered position
            vuint8m1_t acc_vec = __riscv_vle8_v_u8m1(&accumulator[rowOffsetImg + i][colOffsetImg + j], vl);
            
            // Load corresponding image pixel values
            vuint8m1_t img_vec = __riscv_vle8_v_u8m1(&image[i][j], vl);
            
            // Compute squares using widening multiplication for both vectors
            // This expands 8-bit values to 16-bit to prevent intermediate overflow
            vuint16m2_t acc_sqr_vec = __riscv_vwmulu_vv_u16m2(acc_vec, acc_vec, vl);
            vuint16m2_t img_sqr_vec = __riscv_vwmulu_vv_u16m2(img_vec, img_vec, vl);
            
            // Add squared values in 16-bit precision
            vuint16m2_t sum_vec_16 = __riscv_vadd_vv_u16m2(acc_sqr_vec, img_sqr_vec, vl);
            
            vuint8m1_t result;
            
            // Apply overflow handling for final 8-bit conversion
            if (clip == CLIPPING::WRAP) {
                // WRAP mode: Allow overflow to wrap around (modulo 256)
                // Direct narrowing conversion from 16-bit to 8-bit
                result = __riscv_vncvt_x_x_w_u8m1(sum_vec_16, vl);
            } else if (clip == CLIPPING::SATURATION) {
                // SATURATION mode: Clamp values to [0, 255] range
                // Detect overflow condition in 16-bit sum
                vbool8_t overflow = __riscv_vmsgtu_vx_u16m2_b8(sum_vec_16, 255, vl);
                
                // Apply saturation: keep original where no overflow, set 255 where overflow
                vuint16m2_t saturated = __riscv_vmerge_vxm_u16m2(sum_vec_16, 255, overflow, vl);
                
                // Convert saturated values back to 8-bit format
                result = __riscv_vncvt_x_x_w_u8m1(saturated, vl);
            }
            
            // Store final accumulated squared results back to accumulator
            __riscv_vse8_v_u8m1(&accumulator[rowOffsetImg + i][colOffsetImg + j], result, vl);
            
            // Advance to next vector chunk
            j += vl;
        }
    }
}
