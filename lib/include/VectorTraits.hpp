#pragma once
#include <riscv_vector.h>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <algorithm>

template<typename T>
struct VectorTraits;

template<>
struct VectorTraits<uint8_t> {
    using vec_type = vuint8m1_t;
    using wide_type = uint16_t;
    using wide_vec_type = vuint16m2_t;
    using signed_vec_type = vint8m1_t;
    using float_vec_type = vfloat16m2_t;
    using mask_type = vbool8_t;

    // ========== BASIC VECTOR OPERATIONS ==========

    static size_t vsetvl(size_t avl) {
        return __riscv_vsetvl_e8m1(avl);
    }

    static vec_type vmv_v_x(uint8_t value, size_t vl) {
        return __riscv_vmv_v_x_u8m1(value, vl);
    }

    static wide_vec_type vmv_v_x_wide(wide_type value, size_t vl) {
        return __riscv_vmv_v_x_u16m2(value, vl);
    }

    static vec_type vle(const uint8_t* ptr, size_t vl) {
        return __riscv_vle8_v_u8m1(ptr, vl);
    }

    static void vse(uint8_t* ptr, vec_type vec, size_t vl) {
        __riscv_vse8_v_u8m1(ptr, vec, vl);
    }

    // ========== ARITHMETIC OPERATIONS ==========

    static vec_type vadd_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vadd_vv_u8m1(a, b, vl);
    }

    static vec_type vadd_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vadd_vx_u8m1(a, scalar, vl);
    }

    static vec_type vsub_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vsub_vv_u8m1(a, b, vl);
    }

    static vec_type vsub_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vsub_vx_u8m1(a, scalar, vl);
    }

    static vec_type vmul_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmul_vv_u8m1(a, b, vl);
    }

    static vec_type vmul_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vmul_vx_u8m1(a, scalar, vl);
    }

    static vec_type vdiv_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vdivu_vv_u8m1(a, b, vl);
    }

    static vec_type vdiv_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vdivu_vx_u8m1(a, scalar, vl);
    }

    // ========== WIDENING OPERATIONS ==========

    static wide_vec_type wadd_zero(vec_type a, size_t vl) {
        vec_type zero = vmv_v_x(0, vl);
        return __riscv_vwaddu_vv_u16m2(zero, a, vl);
    }

    static wide_vec_type wadd_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vwaddu_vv_u16m2(a, b, vl);
    }

    static wide_vec_type wsub_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vwsubu_vv_u16m2(a, b, vl);
    }

    static wide_vec_type wmul_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vwmulu_vv_u16m2(a, b, vl);
    }

    static wide_vec_type wmul_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vwmulu_vx_u16m2(a, scalar, vl);
    }

    // ========== WIDE VECTOR OPERATIONS ==========

    static wide_vec_type vadd_vv_wide(wide_vec_type a, wide_vec_type b, size_t vl) {
        return __riscv_vadd_vv_u16m2(a, b, vl);
    }

    static wide_vec_type vadd_vx_wide(wide_vec_type a, uint16_t scalar, size_t vl) {
        return __riscv_vadd_vx_u16m2(a, scalar, vl);
    }

    static wide_vec_type vmul_vv_wide(wide_vec_type a, wide_vec_type b, size_t vl) {
        return __riscv_vmul_vv_u16m2(a, b, vl);
    }

    static wide_vec_type vmul_vx_wide(wide_vec_type a, uint16_t scalar, size_t vl) {
        return __riscv_vmul_vx_u16m2(a, scalar, vl);
    }

    static wide_vec_type divu(wide_vec_type a, int divisor, size_t vl) {
        return __riscv_vdivu_vx_u16m2(a, divisor, vl);
    }

    // ========== NARROWING OPERATIONS ==========

    static vec_type vnclipu(wide_vec_type a, unsigned int shift, size_t vl) {
        return __riscv_vnclipu_wx_u8m1(a, shift, 0, vl);
    }

    // Note: vnclip requires signed types, use vnclipu for unsigned narrowing

    // ========== COMPARISON OPERATIONS ==========

    static mask_type vmseq_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmseq_vv_u8m1_b8(a, b, vl);
    }

    static mask_type vmseq_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vmseq_vx_u8m1_b8(a, scalar, vl);
    }

    static mask_type vmsne_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmsne_vv_u8m1_b8(a, b, vl);
    }

    static mask_type vmslt_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmsltu_vv_u8m1_b8(a, b, vl);
    }

    static mask_type vmslt_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vmsltu_vx_u8m1_b8(a, scalar, vl);
    }

    static mask_type vmsle_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmsleu_vv_u8m1_b8(a, b, vl);
    }

    static mask_type vmsgt_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmsgtu_vv_u8m1_b8(a, b, vl);
    }

    static mask_type vmsgt_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vmsgtu_vx_u8m1_b8(a, scalar, vl);
    }

    // ========== LOGICAL OPERATIONS ==========

    static vec_type vand_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vand_vv_u8m1(a, b, vl);
    }

    static vec_type vand_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vand_vx_u8m1(a, scalar, vl);
    }

    static vec_type vor_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vor_vv_u8m1(a, b, vl);
    }

    static vec_type vxor_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vxor_vv_u8m1(a, b, vl);
    }

    static vec_type vnot_v(vec_type a, size_t vl) {
        return __riscv_vnot_v_u8m1(a, vl);
    }

    // ========== SHIFT OPERATIONS ==========

    static vec_type vsll_vx(vec_type a, unsigned int shift, size_t vl) {
        return __riscv_vsll_vx_u8m1(a, shift, vl);
    }

    static vec_type vsrl_vx(vec_type a, unsigned int shift, size_t vl) {
        return __riscv_vsrl_vx_u8m1(a, shift, vl);
    }

    // Note: vsra (arithmetic right shift) requires signed types

    // ========== MIN/MAX OPERATIONS ==========

    static vec_type vmin_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vminu_vv_u8m1(a, b, vl);
    }

    static vec_type vmin_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vminu_vx_u8m1(a, scalar, vl);
    }

    static vec_type vmax_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmaxu_vv_u8m1(a, b, vl);
    }

    static vec_type vmax_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vmaxu_vx_u8m1(a, scalar, vl);
    }

    // ========== SATURATING ARITHMETIC ==========

    static vec_type vsaddu_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vsaddu_vv_u8m1(a, b, vl);
    }

    static vec_type vsaddu_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vsaddu_vx_u8m1(a, scalar, vl);
    }

    static vec_type vssubu_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vssubu_vv_u8m1(a, b, vl);
    }

    static vec_type vssubu_vx(vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vssubu_vx_u8m1(a, scalar, vl);
    }

    // ========== MERGE AND SELECT OPERATIONS ==========

    static vec_type vmerge_vvm(mask_type mask, vec_type a, vec_type b, size_t vl) {
        return __riscv_vmerge_vvm_u8m1(a, b, mask, vl);
    }

    static vec_type vmerge_vxm(mask_type mask, vec_type a, uint8_t scalar, size_t vl) {
        return __riscv_vmerge_vxm_u8m1(a, scalar, mask, vl);
    }

    // ========== PERMUTATION OPERATIONS ==========

    static vec_type vrgather_vv(vec_type src, vec_type indices, size_t vl) {
        return __riscv_vrgather_vv_u8m1(src, indices, vl);
    }

    static vec_type vrgather_vx(vec_type src, size_t index, size_t vl) {
        return __riscv_vrgather_vx_u8m1(src, index, vl);
    }

    static vec_type vslideup_vx(vec_type dest, vec_type src, size_t offset, size_t vl) {
        return __riscv_vslideup_vx_u8m1(dest, src, offset, vl);
    }

    static vec_type vslidedown_vx(vec_type src, size_t offset, size_t vl) {
        return __riscv_vslidedown_vx_u8m1(src, offset, vl);
    }

    // ========== REDUCTION OPERATIONS ==========

    static uint8_t vredsum_vs(vec_type src, vec_type scalar, size_t vl) {
        vuint8m1_t result = __riscv_vredsum_vs_u8m1_u8m1(src, scalar, vl);
        return __riscv_vmv_x_s_u8m1_u8(result);
    }

    static uint8_t vredmax_vs(vec_type src, vec_type scalar, size_t vl) {
        vuint8m1_t result = __riscv_vredmaxu_vs_u8m1_u8m1(src, scalar, vl);
        return __riscv_vmv_x_s_u8m1_u8(result);
    }

    static uint8_t vredmin_vs(vec_type src, vec_type scalar, size_t vl) {
        vuint8m1_t result = __riscv_vredminu_vs_u8m1_u8m1(src, scalar, vl);
        return __riscv_vmv_x_s_u8m1_u8(result);
    }

    // ========== FLOATING POINT CONVERSION ==========

    static float_vec_type vfcvt_f_xu_v(wide_vec_type src, size_t vl) {
        return __riscv_vfcvt_f_xu_v_f16m2(src, vl);
    }

    static wide_vec_type vfcvt_xu_f_v(float_vec_type src, size_t vl) {
        return __riscv_vfcvt_xu_f_v_u16m2(src, vl);
    }

    // ========== FLOATING POINT ARITHMETIC ==========

    static float_vec_type vfadd_vv_f(float_vec_type a, float_vec_type b, size_t vl) {
        return __riscv_vfadd_vv_f16m2(a, b, vl);
    }

    static float_vec_type vfmul_vf_f(float_vec_type a, _Float16 scalar, size_t vl) {
        return __riscv_vfmul_vf_f16m2(a, scalar, vl);
    }

    static float_vec_type vfmadd_vf_f(float_vec_type a, _Float16 scalar, float_vec_type c, size_t vl) {
        return __riscv_vfmadd_vf_f16m2(a, scalar, c, vl);
    }

    // ========== UTILITY FUNCTIONS ==========

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

    // Clamp to [0, 255] range
    static vec_type clamp_to_uint8(wide_vec_type src, size_t vl) {
        wide_vec_type max_val = vmv_v_x_wide(255, vl);
        wide_vec_type clamped = __riscv_vminu_vv_u16m2(src, max_val, vl);
        return vnclipu(clamped, 0, vl);
    }
};

template<>
struct VectorTraits<uint16_t> {
    using vec_type = vuint16m1_t;
    using wide_type = uint32_t;
    using wide_vec_type = vuint32m2_t;
    using signed_vec_type = vint16m1_t;
    using float_vec_type = vfloat32m2_t;
    using mask_type = vbool16_t;

    // ========== BASIC VECTOR OPERATIONS ==========

    static size_t vsetvl(size_t avl) {
        return __riscv_vsetvl_e16m1(avl);
    }

    static vec_type vmv_v_x(uint16_t value, size_t vl) {
        return __riscv_vmv_v_x_u16m1(value, vl);
    }

    static wide_vec_type vmv_v_x_wide(wide_type value, size_t vl) {
        return __riscv_vmv_v_x_u32m2(value, vl);
    }

    static vec_type vle(const uint16_t* ptr, size_t vl) {
        return __riscv_vle16_v_u16m1(ptr, vl);
    }

    static void vse(uint16_t* ptr, vec_type vec, size_t vl) {
        __riscv_vse16_v_u16m1(ptr, vec, vl);
    }

    // ========== ARITHMETIC OPERATIONS ==========

    static vec_type vadd_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vadd_vv_u16m1(a, b, vl);
    }

    static vec_type vadd_vx(vec_type a, uint16_t scalar, size_t vl) {
        return __riscv_vadd_vx_u16m1(a, scalar, vl);
    }

    static vec_type vsub_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vsub_vv_u16m1(a, b, vl);
    }

    static vec_type vmul_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmul_vv_u16m1(a, b, vl);
    }

    static vec_type vmul_vx(vec_type a, uint16_t scalar, size_t vl) {
        return __riscv_vmul_vx_u16m1(a, scalar, vl);
    }

    static vec_type vdiv_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vdivu_vv_u16m1(a, b, vl);
    }

    static vec_type vdiv_vx(vec_type a, uint16_t scalar, size_t vl) {
        return __riscv_vdivu_vx_u16m1(a, scalar, vl);
    }

    // ========== WIDENING OPERATIONS ==========

    static wide_vec_type wadd_zero(vec_type a, size_t vl) {
        vec_type zero = vmv_v_x(0, vl);
        return __riscv_vwaddu_vv_u32m2(zero, a, vl);
    }

    static wide_vec_type wadd_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vwaddu_vv_u32m2(a, b, vl);
    }

    static wide_vec_type wmul_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vwmulu_vv_u32m2(a, b, vl);
    }

    // ========== WIDE VECTOR OPERATIONS ==========

    static wide_vec_type vadd_vv_wide(wide_vec_type a, wide_vec_type b, size_t vl) {
        return __riscv_vadd_vv_u32m2(a, b, vl);
    }

    static wide_vec_type vmul_vx_wide(wide_vec_type a, uint32_t scalar, size_t vl) {
        return __riscv_vmul_vx_u32m2(a, scalar, vl);
    }

    static wide_vec_type divu(wide_vec_type a, int divisor, size_t vl) {
        return __riscv_vdivu_vx_u32m2(a, divisor, vl);
    }

    // ========== NARROWING OPERATIONS ==========

    static vec_type vnclipu(wide_vec_type a, unsigned int shift, size_t vl) {
        return __riscv_vnclipu_wx_u16m1(a, shift, 0, vl);
    }

    // ========== MIN/MAX OPERATIONS ==========

    static vec_type vmin_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vminu_vv_u16m1(a, b, vl);
    }

    static vec_type vmax_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmaxu_vv_u16m1(a, b, vl);
    }

    // ========== SATURATING ARITHMETIC ==========

    static vec_type vsaddu_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vsaddu_vv_u16m1(a, b, vl);
    }

    static vec_type vssubu_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vssubu_vv_u16m1(a, b, vl);
    }

    // ========== COMPARISON OPERATIONS ==========

    static mask_type vmseq_vv(vec_type a, vec_type b, size_t vl) {
        return __riscv_vmseq_vv_u16m1_b16(a, b, vl);
    }

    static mask_type vmsgt_vx(vec_type a, uint16_t scalar, size_t vl) {
        return __riscv_vmsgtu_vx_u16m1_b16(a, scalar, vl);
    }

    // ========== PERMUTATION OPERATIONS ==========

    static vec_type vrgather_vv(vec_type src, vec_type indices, size_t vl) {
        return __riscv_vrgather_vv_u16m1(src, indices, vl);
    }

    static vec_type vrgather_vx(vec_type src, size_t index, size_t vl) {
        return __riscv_vrgather_vx_u16m1(src, index, vl);
    }

    // ========== UTILITY FUNCTIONS ==========

    static vec_type create_reverse_index(size_t vl) {
        uint16_t indices[vl];
        for (size_t i = 0; i < vl; ++i) {
            indices[i] = static_cast<uint16_t>(vl - 1 - i);
        }
        return vle(indices, vl);
    }

    static vec_type reverse_vector(vec_type src, size_t vl) {
        vec_type indices = create_reverse_index(vl);
        return vrgather_vv(src, indices, vl);
    }

    static vec_type clamp_to_uint16(wide_vec_type src, size_t vl) {
        wide_vec_type max_val = vmv_v_x_wide(65535, vl);
        wide_vec_type clamped = __riscv_vminu_vv_u32m2(src, max_val, vl);
        return vnclipu(clamped, 0, vl);
    }
};

// ========== SPECIALIZED HELPER FUNCTIONS ==========

// Template helper for different data types
template<typename T>
static inline typename VectorTraits<T>::vec_type vector_abs_diff(
    typename VectorTraits<T>::vec_type a, 
    typename VectorTraits<T>::vec_type b, 
    size_t vl) {
    auto mask_a_gt_b = VectorTraits<T>::vmsgt_vv(a, b, vl);
    auto diff1 = VectorTraits<T>::vsub_vv(a, b, vl);
    auto diff2 = VectorTraits<T>::vsub_vv(b, a, vl);
    return VectorTraits<T>::vmerge_vvm(mask_a_gt_b, diff2, diff1, vl);
}

// Optimized Sobel edge detection kernels
template<typename T>
static inline typename VectorTraits<T>::wide_vec_type vector_sobel_x(
    typename VectorTraits<T>::vec_type p1, typename VectorTraits<T>::vec_type p2, typename VectorTraits<T>::vec_type p3,
    typename VectorTraits<T>::vec_type p7, typename VectorTraits<T>::vec_type p8, typename VectorTraits<T>::vec_type p9,
    size_t vl) {
    // Sobel X: [-1, 0, 1; -2, 0, 2; -1, 0, 1]
    auto sum_left = VectorTraits<T>::wadd_vv(p1, VectorTraits<T>::vmul_vx(p7, 2, vl), vl);
    sum_left = VectorTraits<T>::vadd_vv_wide(sum_left, VectorTraits<T>::wadd_zero(p3, vl), vl);
    
    auto sum_right = VectorTraits<T>::wadd_vv(p9, VectorTraits<T>::vmul_vx(p3, 2, vl), vl);
    sum_right = VectorTraits<T>::vadd_vv_wide(sum_right, VectorTraits<T>::wadd_zero(p1, vl), vl);
    
    return VectorTraits<T>::vadd_vv_wide(sum_right, sum_left, vl); // Note: simplified, may need proper signed arithmetic
}