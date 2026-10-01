#ifndef RISC201_LOGIC_GATES_HPP
#define RISC201_LOGIC_GATES_HPP

#include "../common/Types.hpp"

namespace risc201 {

/**
 * Gate-level logic primitives constructed according to Sarangi Chapter 7.
 * To avoid high-level operator shortcuts, logic operations and adders
 * are synthesized directly from boolean gates and transmission-level concepts.
 */
namespace gates {

// Single-bit primitive gates
inline uint8_t g_not(uint8_t a) {
    return (a & 1) ? 0 : 1;
}

inline uint8_t g_and(uint8_t a, uint8_t b) {
    return ((a & 1) && (b & 1)) ? 1 : 0;
}

inline uint8_t g_or(uint8_t a, uint8_t b) {
    return ((a & 1) || (b & 1)) ? 1 : 0;
}

inline uint8_t g_nand(uint8_t a, uint8_t b) {
    return g_not(g_and(a, b));
}

inline uint8_t g_nor(uint8_t a, uint8_t b) {
    return g_not(g_or(a, b));
}

// XOR synthesized from AND, OR, NOT: A*~B + ~A*B (Sarangi Section 7.2.1, Figure 7.8)
inline uint8_t g_xor(uint8_t a, uint8_t b) {
    uint8_t term1 = g_and(a, g_not(b));
    uint8_t term2 = g_and(g_not(a), b);
    return g_or(term1, term2);
}

// 2:1 Multiplexer (Sarangi Section 7.2.3, Figure 7.11):
// Out = (s * in1) + (~s * in0)
inline uint8_t g_mux2(uint8_t s, uint8_t in0, uint8_t in1) {
    return g_or(g_and(g_not(s), in0), g_and(s, in1));
}

// 1-bit Half Adder (Sarangi Section 8.1.1, Figure 8.1)
// sum = a XOR b, carry = a AND b
inline void half_adder(uint8_t a, uint8_t b, uint8_t& sum, uint8_t& cout) {
    sum = g_xor(a, b);
    cout = g_and(a, b);
}

// 1-bit Full Adder (Sarangi Section 8.1.2, Figure 8.2)
// sum = a XOR b XOR cin
// cout = (a * b) + (cin * (a XOR b))  or  (a*b) + (a*cin) + (b*cin)
inline void full_adder(uint8_t a, uint8_t b, uint8_t cin, uint8_t& sum, uint8_t& cout) {
    uint8_t a_xor_b = g_xor(a, b);
    sum = g_xor(a_xor_b, cin);
    uint8_t term1 = g_and(a, b);
    uint8_t term2 = g_and(cin, a_xor_b);
    cout = g_or(term1, term2);
}

// Bit extraction helpers
inline uint8_t get_bit(Word val, int bit_idx) {
    return static_cast<uint8_t>((val >> bit_idx) & 1);
}

inline void set_bit(Word& val, int bit_idx, uint8_t b) {
    if (b & 1) {
        val |= (1u << bit_idx);
    } else {
        val &= ~(1u << bit_idx);
    }
}

// 32-bit Bitwise Logical Operations synthesized gate-by-gate
inline Word bitwise_and(Word a, Word b) {
    Word res = 0;
    for (int i = 0; i < 32; ++i) {
        set_bit(res, i, g_and(get_bit(a, i), get_bit(b, i)));
    }
    return res;
}

inline Word bitwise_or(Word a, Word b) {
    Word res = 0;
    for (int i = 0; i < 32; ++i) {
        set_bit(res, i, g_or(get_bit(a, i), get_bit(b, i)));
    }
    return res;
}

inline Word bitwise_xor(Word a, Word b) {
    Word res = 0;
    for (int i = 0; i < 32; ++i) {
        set_bit(res, i, g_xor(get_bit(a, i), get_bit(b, i)));
    }
    return res;
}

inline Word bitwise_not(Word a) {
    Word res = 0;
    for (int i = 0; i < 32; ++i) {
        set_bit(res, i, g_not(get_bit(a, i)));
    }
    return res;
}

// 32-bit 2:1 Multiplexer
inline Word mux2_32(uint8_t s, Word in0, Word in1) {
    Word res = 0;
    for (int i = 0; i < 32; ++i) {
        set_bit(res, i, g_mux2(s, get_bit(in0, i), get_bit(in1, i)));
    }
    return res;
}

} // namespace gates
} // namespace risc201

#endif // RISC201_LOGIC_GATES_HPP
