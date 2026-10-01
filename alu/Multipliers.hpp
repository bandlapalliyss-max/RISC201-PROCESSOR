#ifndef RISC201_MULTIPLIERS_HPP
#define RISC201_MULTIPLIERS_HPP

#include "LogicGates.hpp"
#include "Adders.hpp"
#include <vector>

namespace risc201 {
namespace alu {

/**
 * 65-bit Register UV helper for Booth and Iterative Multipliers.
 * U is 33 bits [64..32], V is 32 bits [31..0].
 * Implements arithmetic right shift preserving sign bit (bit 64).
 */
struct RegUV {
    uint64_t u{0}; // 33-bit U (bits 0 to 32)
    uint32_t v{0}; // 32-bit V (bits 0 to 31)

    // Arithmetic right shift by 1 position across UV
    void arithmeticRightShift() {
        uint8_t signU = (u >> 32) & 1; // Bit 32 of U is sign bit
        uint8_t lsbU = u & 1;

        u = (u >> 1) | (static_cast<uint64_t>(signU) << 32);
        v = (v >> 1) | (static_cast<uint32_t>(lsbU) << 31);
    }
};

/**
 * 33-bit addition/subtraction helper using gate logic for U and N.
 */
inline uint64_t add33(uint64_t u, uint64_t n, uint8_t cin = 0) {
    uint64_t sum = 0;
    uint8_t carry = cin;
    for (int i = 0; i < 33; ++i) {
        uint8_t a = (u >> i) & 1;
        uint8_t b = (n >> i) & 1;
        uint8_t s = 0;
        uint8_t cout = 0;
        gates::full_adder(a, b, carry, s, cout);
        if (s) sum |= (1ULL << i);
        carry = cout;
    }
    return sum;
}

inline uint64_t sub33(uint64_t u, uint64_t n) {
    uint64_t notN = 0;
    for (int i = 0; i < 33; ++i) {
        if (!((n >> i) & 1)) notN |= (1ULL << i);
    }
    return add33(u, notN, 1);
}

/**
 * 1. Iterative Multiplier (Sarangi Section 8.2.2, Algorithm 1)
 * Multiplies two signed 32-bit numbers into a 64-bit product.
 * Time Complexity: O(n log n)
 */
inline uint64_t iterativeMultiply(Word multiplicand, Word multiplier) {
    RegUV uv;
    uv.u = 0;
    uv.v = multiplier;

    // N is 33 bits with sign extension from bit 31
    uint64_t n = multiplicand;
    if (multiplicand & 0x80000000) {
        n |= (1ULL << 32); // Sign extend to 33 bits
    }

    for (int i = 1; i <= 32; ++i) {
        if (uv.v & 1) { // LSB of V is 1
            if (i < 32) {
                uv.u = add33(uv.u, n);
            } else {
                // MSB of 2's complement multiplier: subtract N (Theorem 2.3.4.2)
                uv.u = sub33(uv.u, n);
            }
        }
        uv.arithmeticRightShift();
    }

    // Lower 64 bits of UV contain the result
    return (uv.u << 32) | uv.v;
}

/**
 * 2. Radix-2 Booth's Multiplier (Sarangi Section 8.2.3, Algorithm 2)
 * Checks bit pair (currBit, prevBit):
 * (1, 0) -> U = U - N (start of run of 1s)
 * (0, 1) -> U = U + N (end of run of 1s)
 * (0, 0) or (1, 1) -> nop
 * Followed by arithmetic right shift.
 */
inline uint64_t boothMultiply(Word multiplicand, Word multiplier) {
    RegUV uv;
    uv.u = 0;
    uv.v = multiplier;
    uint8_t prevBit = 0;

    uint64_t n = multiplicand;
    if (multiplicand & 0x80000000) {
        n |= (1ULL << 32);
    }

    for (int i = 1; i <= 32; ++i) {
        uint8_t currBit = uv.v & 1;
        if (currBit == 1 && prevBit == 0) {
            uv.u = sub33(uv.u, n);
        } else if (currBit == 0 && prevBit == 1) {
            uv.u = add33(uv.u, n);
        }
        prevBit = currBit;
        uv.arithmeticRightShift();
    }

    return (uv.u << 32) | uv.v;
}

/**
 * 3. Wallace Tree Multiplier (Sarangi Section 8.2.5, Figure 8.14)
 * Uses Carry Save Adders (CSA) to reduce 32 partial products down to 2 vectors,
 * then adds the final two vectors using Carry Lookahead Adder.
 * Time Complexity: O(log n)
 */
inline uint64_t wallaceTreeMultiply(Word a, Word b) {
    // Generate 32 partial products using bitwise AND
    // Each partial product is 64-bit wide
    std::vector<uint64_t> partialProducts(32, 0);
    for (int i = 0; i < 32; ++i) {
        uint8_t b_bit = gates::get_bit(b, i);
        if (b_bit) {
            partialProducts[i] = static_cast<uint64_t>(a) << i;
        } else {
            partialProducts[i] = 0;
        }
    }

    // Carry Save Adder reduction: A + B + C = Sum + (Carry << 1)
    auto csa_reduce = [](uint64_t x, uint64_t y, uint64_t z, uint64_t& sum, uint64_t& carry) {
        sum = 0;
        carry = 0;
        for (int i = 0; i < 64; ++i) {
            uint8_t bit_x = (x >> i) & 1;
            uint8_t bit_y = (y >> i) & 1;
            uint8_t bit_z = (z >> i) & 1;
            uint8_t s = 0, c = 0;
            gates::full_adder(bit_x, bit_y, bit_z, s, c);
            if (s) sum |= (1ULL << i);
            if (c) carry |= (1ULL << i);
        }
        carry <<= 1;
    };

    // Reduce list of vectors until only 2 remain
    std::vector<uint64_t> current = partialProducts;
    while (current.size() > 2) {
        std::vector<uint64_t> nextLevel;
        size_t i = 0;
        while (i + 2 < current.size()) {
            uint64_t sum = 0, carry = 0;
            csa_reduce(current[i], current[i+1], current[i+2], sum, carry);
            nextLevel.push_back(sum);
            nextLevel.push_back(carry);
            i += 3;
        }
        while (i < current.size()) {
            nextLevel.push_back(current[i]);
            ++i;
        }
        current = nextLevel;
    }

    if (current.size() == 1) return current[0];

    // Final 64-bit addition of the remaining two vectors using CLA
    Word sumLow = 0, sumHigh = 0;
    uint8_t carry = 0;
    bool ovf = false;
    std::tie(sumLow, carry, ovf) = carryLookaheadAdd(static_cast<Word>(current[0]), static_cast<Word>(current[1]), 0);
    std::tie(sumHigh, carry, ovf) = carryLookaheadAdd(static_cast<Word>(current[0] >> 32), static_cast<Word>(current[1] >> 32), carry);

    return (static_cast<uint64_t>(sumHigh) << 32) | sumLow;
}

} // namespace alu
} // namespace risc201

#endif // RISC201_MULTIPLIERS_HPP
