#ifndef RISC201_DIVIDERS_HPP
#define RISC201_DIVIDERS_HPP

#include "LogicGates.hpp"
#include "Adders.hpp"
#include "../common/Exception.hpp"
#include <tuple>

namespace risc201 {
namespace alu {

/**
 * 65-bit Register UV helper for division algorithms.
 * U is 33 bits, V is 32 bits.
 * Left shift shifts MSB of V into LSB of U.
 */
struct DivRegUV {
    int64_t u{0};  // 33-bit signed U
    uint32_t v{0}; // 32-bit unsigned V

    void leftShift() {
        uint8_t msbV = (v >> 31) & 1;
        // Shift U left by 1 and insert msb of V into bit 0
        u = ((u << 1) & 0x1FFFFFFFFULL) | msbV;
        // Sign-extend bit 32 if negative in 33-bit representation
        if (u & (1ULL << 32)) {
            u |= ~0x1FFFFFFFFULL;
        }
        v = (v << 1);
    }

    void setQuotientBit(uint8_t q) {
        if (q & 1) {
            v |= 1u;
        } else {
            v &= ~1u;
        }
    }
};

/**
 * 1. Restoring Division Algorithm (Sarangi Section 8.3.2, Algorithm 3)
 * Divides positive 32-bit numbers: N (dividend) by D (divisor).
 * In each cycle:
 *   UV << 1
 *   U = U - D
 *   if (U >= 0) q = 1
 *   else U = U + D (restore!), q = 0
 *   LSB(V) = q
 */
inline std::tuple<Word, Word, bool> restoringDivideUnsigned(Word dividend, Word divisor) {
    if (divisor == 0) {
        return {0, 0, true}; // Division by zero flag
    }

    DivRegUV uv;
    uv.u = 0;
    uv.v = dividend;
    int64_t d = divisor;

    for (int i = 0; i < 32; ++i) {
        uv.leftShift();
        uv.u -= d;
        if (uv.u >= 0) {
            uv.setQuotientBit(1);
        } else {
            uv.u += d; // Restore
            uv.setQuotientBit(0);
        }
    }

    Word quotient = uv.v;
    Word remainder = static_cast<Word>(uv.u & 0xFFFFFFFF);
    return {quotient, remainder, false};
}

/**
 * 2. Non-Restoring Division Algorithm (Sarangi Section 8.3.3, Algorithm 4)
 * Eliminates the restoring addition on trial failure.
 * In each cycle:
 *   UV << 1
 *   if (U >= 0) U = U - D;
 *   else U = U + D;
 *   if (U >= 0) q = 1;
 *   else q = 0;
 *   LSB(V) = q
 * Final correction: if U < 0, U = U + D
 */
inline std::tuple<Word, Word, bool> nonRestoringDivideUnsigned(Word dividend, Word divisor) {
    if (divisor == 0) {
        return {0, 0, true}; // Division by zero flag
    }

    DivRegUV uv;
    uv.u = 0;
    uv.v = dividend;
    int64_t d = divisor;

    for (int i = 0; i < 32; ++i) {
        uv.leftShift();
        if (uv.u >= 0) {
            uv.u -= d;
        } else {
            uv.u += d;
        }

        if (uv.u >= 0) {
            uv.setQuotientBit(1);
        } else {
            uv.setQuotientBit(0);
        }
    }

    // Final correction if remainder is negative
    if (uv.u < 0) {
        uv.u += d;
    }

    Word quotient = uv.v;
    Word remainder = static_cast<Word>(uv.u & 0xFFFFFFFF);
    return {quotient, remainder, false};
}

/**
 * Signed Division Wrapper (Sarangi Section 8.3.1):
 * Converts negative inputs to 2's complement magnitudes,
 * performs division via restoring or non-restoring algorithm,
 * and fixes signs (truncation towards zero).
 */
inline std::tuple<Word, Word, bool> divideSigned(Word n, Word d, bool useNonRestoring = true) {
    if (d == 0) {
        return {0, 0, true}; // Division by zero
    }

    bool nNeg = (n & 0x80000000) != 0;
    bool dNeg = (d & 0x80000000) != 0;

    // Convert to positive magnitudes via 2's complement (~X + 1)
    Word absN = nNeg ? std::get<0>(subtract32(0, n)) : n;
    Word absD = dNeg ? std::get<0>(subtract32(0, d)) : d;

    auto [quot, rem, divZero] = useNonRestoring ?
        nonRestoringDivideUnsigned(absN, absD) :
        restoringDivideUnsigned(absN, absD);

    // Quotient is negative if sign(N) != sign(D)
    if (nNeg ^ dNeg) {
        quot = std::get<0>(subtract32(0, quot));
    }

    // Remainder has the same sign as dividend N (Sarangi page 334)
    if (nNeg) {
        rem = std::get<0>(subtract32(0, rem));
    }

    return {quot, rem, false};
}

} // namespace alu
} // namespace risc201

#endif // RISC201_DIVIDERS_HPP
