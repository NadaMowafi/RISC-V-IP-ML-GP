#pragma once
#include <riscv_vector.h>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <algorithm>
#include <vector>

// ========== LMUL VARIANT TRAITS ==========
// Template parameter: LMUL (1, 2, 4, 8)
template<typename T, int LMUL>
struct VectorTraits_LMUL;

// ========== UINT8_T SPECIALIZATIONS ==========

template<>
struct VectorTraits_LMUL<uint8_t, 1> {
    using vec_type = vuint8m1_t;
    using wide_type = uint16_t;
    using wide_vec_type = vuint16m2_t;
    using mask_type = vbool8_t;
    
    static constexpr int lmul = 1;
    static constexpr const char* lmul_str = "m1";
    
    static size_t vsetvl(size_t avl) { return __riscv_vsetvl_e8m1(avl); }
    static vec_type vmv_v_x(uint8_t value, size_t vl) { return __riscv_vmv_v_x_u8m1(value, vl); }
    static wide_vec_type vmv_v_x_wide(wide_type value, size_t vl) { return __riscv_vmv_v_x_u16m2(value, vl); }
    static vec_type vle(const uint8_t* ptr, size_t vl) { return __riscv_vle8_v_u8m1(ptr, vl); }
    static void vse(uint8_t* ptr, vec_type vec, size_t vl) { __riscv_vse8_v_u8m1(ptr, vec, vl); }
    static vec_type vadd_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vadd_vv_u8m1(a, b, vl); }
    static vec_type vadd_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vadd_vx_u8m1(a, scalar, vl); }
    static vec_type vsub_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vsub_vv_u8m1(a, b, vl); }
    static vec_type vsub_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vsub_vx_u8m1(a, scalar, vl); }
    static vec_type vmul_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vmul_vv_u8m1(a, b, vl); }
    static vec_type vmul_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vmul_vx_u8m1(a, scalar, vl); }
    static vec_type vdiv_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vdivu_vv_u8m1(a, b, vl); }
    static vec_type vmin_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vminu_vv_u8m1(a, b, vl); }
    static vec_type vmax_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vmaxu_vv_u8m1(a, b, vl); }
    static vec_type vsaddu_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vsaddu_vv_u8m1(a, b, vl); }
    static vec_type vrgather_vv(vec_type src, vec_type indices, size_t vl) { return __riscv_vrgather_vv_u8m1(src, indices, vl); }
    static wide_vec_type wadd_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vwaddu_vv_u16m2(a, b, vl); }
    static wide_vec_type wmul_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vwmulu_vv_u16m2(a, b, vl); }
    static vec_type vnclipu(wide_vec_type a, unsigned int shift, size_t vl) { return __riscv_vnclipu_wx_u8m1(a, shift, 0, vl); }
    
    static vec_type create_reverse_index(size_t vl) {
        uint8_t indices[vl];
        for (size_t i = 0; i < vl; ++i) {
            indices[i] = static_cast<uint8_t>(vl - 1 - i);
        }
        return vle(indices, vl);
    }
    
    static vec_type reverse_vector(vec_type src, size_t vl) {
        vec_type indices = create_reverse_index(vl);
        return vrgather_vv(src, indices, vl);
    }
};

template<>
struct VectorTraits_LMUL<uint8_t, 2> {
    using vec_type = vuint8m2_t;
    using wide_type = uint16_t;
    using wide_vec_type = vuint16m4_t;
    using mask_type = vbool4_t;
    
    static constexpr int lmul = 2;
    static constexpr const char* lmul_str = "m2";
    
    static size_t vsetvl(size_t avl) { return __riscv_vsetvl_e8m2(avl); }
    static vec_type vmv_v_x(uint8_t value, size_t vl) { return __riscv_vmv_v_x_u8m2(value, vl); }
    static wide_vec_type vmv_v_x_wide(wide_type value, size_t vl) { return __riscv_vmv_v_x_u16m4(value, vl); }
    static vec_type vle(const uint8_t* ptr, size_t vl) { return __riscv_vle8_v_u8m2(ptr, vl); }
    static void vse(uint8_t* ptr, vec_type vec, size_t vl) { __riscv_vse8_v_u8m2(ptr, vec, vl); }
    static vec_type vadd_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vadd_vv_u8m2(a, b, vl); }
    static vec_type vadd_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vadd_vx_u8m2(a, scalar, vl); }
    static vec_type vsub_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vsub_vv_u8m2(a, b, vl); }
    static vec_type vsub_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vsub_vx_u8m2(a, scalar, vl); }
    static vec_type vmul_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vmul_vv_u8m2(a, b, vl); }
    static vec_type vmul_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vmul_vx_u8m2(a, scalar, vl); }
    static vec_type vdiv_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vdivu_vv_u8m2(a, b, vl); }
    static vec_type vmin_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vminu_vv_u8m2(a, b, vl); }
    static vec_type vmax_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vmaxu_vv_u8m2(a, b, vl); }
    static vec_type vsaddu_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vsaddu_vv_u8m2(a, b, vl); }
    static vec_type vrgather_vv(vec_type src, vec_type indices, size_t vl) { return __riscv_vrgather_vv_u8m2(src, indices, vl); }
    static wide_vec_type wadd_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vwaddu_vv_u16m4(a, b, vl); }
    static wide_vec_type wmul_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vwmulu_vv_u16m4(a, b, vl); }
    static vec_type vnclipu(wide_vec_type a, unsigned int shift, size_t vl) { return __riscv_vnclipu_wx_u8m2(a, shift, 0, vl); }
    
    static vec_type create_reverse_index(size_t vl) {
        uint8_t indices[vl];
        for (size_t i = 0; i < vl; ++i) {
            indices[i] = static_cast<uint8_t>(vl - 1 - i);
        }
        return vle(indices, vl);
    }
    
    static vec_type reverse_vector(vec_type src, size_t vl) {
        vec_type indices = create_reverse_index(vl);
        return vrgather_vv(src, indices, vl);
    }
};

template<>
struct VectorTraits_LMUL<uint8_t, 4> {
    using vec_type = vuint8m4_t;
    using wide_type = uint16_t;
    using wide_vec_type = vuint16m8_t;
    using mask_type = vbool2_t;
    
    static constexpr int lmul = 4;
    static constexpr const char* lmul_str = "m4";
    
    static size_t vsetvl(size_t avl) { return __riscv_vsetvl_e8m4(avl); }
    static vec_type vmv_v_x(uint8_t value, size_t vl) { return __riscv_vmv_v_x_u8m4(value, vl); }
    static wide_vec_type vmv_v_x_wide(wide_type value, size_t vl) { return __riscv_vmv_v_x_u16m8(value, vl); }
    static vec_type vle(const uint8_t* ptr, size_t vl) { return __riscv_vle8_v_u8m4(ptr, vl); }
    static void vse(uint8_t* ptr, vec_type vec, size_t vl) { __riscv_vse8_v_u8m4(ptr, vec, vl); }
    static vec_type vadd_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vadd_vv_u8m4(a, b, vl); }
    static vec_type vadd_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vadd_vx_u8m4(a, scalar, vl); }
    static vec_type vsub_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vsub_vv_u8m4(a, b, vl); }
    static vec_type vsub_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vsub_vx_u8m4(a, scalar, vl); }
    static vec_type vmul_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vmul_vv_u8m4(a, b, vl); }
    static vec_type vmul_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vmul_vx_u8m4(a, scalar, vl); }
    static vec_type vdiv_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vdivu_vv_u8m4(a, b, vl); }
    static vec_type vmin_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vminu_vv_u8m4(a, b, vl); }
    static vec_type vmax_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vmaxu_vv_u8m4(a, b, vl); }
    static vec_type vsaddu_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vsaddu_vv_u8m4(a, b, vl); }
    static vec_type vrgather_vv(vec_type src, vec_type indices, size_t vl) { return __riscv_vrgather_vv_u8m4(src, indices, vl); }
    static wide_vec_type wadd_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vwaddu_vv_u16m8(a, b, vl); }
    static wide_vec_type wmul_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vwmulu_vv_u16m8(a, b, vl); }
    static vec_type vnclipu(wide_vec_type a, unsigned int shift, size_t vl) { return __riscv_vnclipu_wx_u8m4(a, shift, 0, vl); }
    
    static vec_type create_reverse_index(size_t vl) {
        uint8_t indices[vl];
        for (size_t i = 0; i < vl; ++i) {
            indices[i] = static_cast<uint8_t>(vl - 1 - i);
        }
        return vle(indices, vl);
    }
    
    static vec_type reverse_vector(vec_type src, size_t vl) {
        vec_type indices = create_reverse_index(vl);
        return vrgather_vv(src, indices, vl);
    }
};

template<>
struct VectorTraits_LMUL<uint8_t, 8> {
    using vec_type = vuint8m8_t;
    using wide_type = uint16_t;
    using wide_vec_type = vuint16m8_t;  // Note: m8 widening limited by register constraints
    using mask_type = vbool1_t;
    
    static constexpr int lmul = 8;
    static constexpr const char* lmul_str = "m8";
    
    static size_t vsetvl(size_t avl) { return __riscv_vsetvl_e8m8(avl); }
    static vec_type vmv_v_x(uint8_t value, size_t vl) { return __riscv_vmv_v_x_u8m8(value, vl); }
    static wide_vec_type vmv_v_x_wide(wide_type value, size_t vl) { return __riscv_vmv_v_x_u16m8(value, vl); }
    static vec_type vle(const uint8_t* ptr, size_t vl) { return __riscv_vle8_v_u8m8(ptr, vl); }
    static void vse(uint8_t* ptr, vec_type vec, size_t vl) { __riscv_vse8_v_u8m8(ptr, vec, vl); }
    static vec_type vadd_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vadd_vv_u8m8(a, b, vl); }
    static vec_type vadd_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vadd_vx_u8m8(a, scalar, vl); }
    static vec_type vsub_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vsub_vv_u8m8(a, b, vl); }
    static vec_type vsub_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vsub_vx_u8m8(a, scalar, vl); }
    static vec_type vmul_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vmul_vv_u8m8(a, b, vl); }
    static vec_type vmul_vx(vec_type a, uint8_t scalar, size_t vl) { return __riscv_vmul_vx_u8m8(a, scalar, vl); }
    static vec_type vdiv_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vdivu_vv_u8m8(a, b, vl); }
    static vec_type vmin_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vminu_vv_u8m8(a, b, vl); }
    static vec_type vmax_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vmaxu_vv_u8m8(a, b, vl); }
    static vec_type vsaddu_vv(vec_type a, vec_type b, size_t vl) { return __riscv_vsaddu_vv_u8m8(a, b, vl); }
    static vec_type vrgather_vv(vec_type src, vec_type indices, size_t vl) { return __riscv_vrgather_vv_u8m8(src, indices, vl); }
    
    // Note: m8 widening operations may be limited or unavailable
    static vec_type create_reverse_index(size_t vl) {
        uint8_t indices[vl];
        for (size_t i = 0; i < vl; ++i) {
            indices[i] = static_cast<uint8_t>(vl - 1 - i);
        }
        return vle(indices, vl);
    }
    
    static vec_type reverse_vector(vec_type src, size_t vl) {
        vec_type indices = create_reverse_index(vl);
        return vrgather_vv(src, indices, vl);
    }
};

// ========== HELPER TEMPLATES FOR LMUL TESTING ==========

template<int LMUL>
using VectorTraits_uint8_LMUL = VectorTraits_LMUL<uint8_t, LMUL>;

template<int LMUL>
using VectorTraits_uint16_LMUL = VectorTraits_LMUL<uint16_t, LMUL>;

// Template function for generic LMUL operations
template<typename T, int LMUL>
std::vector<std::vector<T>> simple_add_LMUL(const std::vector<std::vector<T>>& img1, 
                                             const std::vector<std::vector<T>>& img2) {
    const int height = img1.size();
    const int width = img1[0].size();
    std::vector<std::vector<T>> result(height, std::vector<T>(width));
    
    using Traits = VectorTraits_LMUL<T, LMUL>;
    
    for (int i = 0; i < height; ++i) {
        int j = 0;
        while (j < width) {
            size_t vl = Traits::vsetvl(width - j);
            auto va = Traits::vle(&img1[i][j], vl);
            auto vb = Traits::vle(&img2[i][j], vl);
            auto vsum = Traits::vsaddu_vv(va, vb, vl);  // Saturating add
            Traits::vse(&result[i][j], vsum, vl);
            j += vl;
        }
    }
    return result;
}

template<typename T, int LMUL>
std::vector<std::vector<T>> simple_flip_horizontal_LMUL(const std::vector<std::vector<T>>& img) {
    const int height = img.size();
    const int width = img[0].size();
    std::vector<std::vector<T>> result(height, std::vector<T>(width));
    
    using Traits = VectorTraits_LMUL<T, LMUL>;
    
    for (int i = 0; i < height; ++i) {
        int j = 0;
        while (j < width) {
            size_t vl = Traits::vsetvl(width - j);
            if (j + vl > width) vl = width - j;
            
            auto vec = Traits::vle(&img[i][j], vl);
            auto reversed = Traits::reverse_vector(vec, vl);
            Traits::vse(&result[i][width - j - vl], reversed, vl);
            j += vl;
        }
    }
    return result;
} 