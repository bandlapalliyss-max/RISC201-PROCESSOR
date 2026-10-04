#ifndef RISC201_SHIFTER_HPP
#define RISC201_SHIFTER_HPP

#include "LogicGates.hpp"

namespace risc201 {
namespace alu {

/**
 * 32-bit Logarithmic Barrel Shifter (Sarangi Section 4.2.2 & 7.33).
 * Synthesized using 5 stages of 2:1 multiplexers:
 *   Stage 4: conditional shift by 16 bits
 *   Stage 3: conditional shift by 8 bits
 *   Stage 2: conditional shift by 4 bits
 *   Stage 1: conditional shift by 2 bits
 *   Stage 0: conditional shift by 1 bit
 * Avoids language built-in << and >> for the core data path!
 */
class BarrelShifter {
public:
    enum class ShiftType {
        LSL, // Logical Shift Left
        LSR, // Logical Shift Right
        ASR  // Arithmetic Shift Right (sign-preserving)
    };

    static Word shift(Word val, Word shiftAmount, ShiftType type) {
        // SimpleRisc shift amount is modulo 32 (only bottom 5 bits matter)
        uint8_t shamt = static_cast<uint8_t>(shiftAmount & 0x1F);
        if (shamt == 0) return val;

        uint8_t signBit = gates::get_bit(val, 31);
        uint8_t fillBit = (type == ShiftType::ASR) ? signBit : 0;

        Word current = val;

        if (type == ShiftType::LSL) {
            // Stage 4: Shift by 16
            if (shamt & 16) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i >= 16) ? gates::get_bit(current, i - 16) : 0;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
            // Stage 3: Shift by 8
            if (shamt & 8) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i >= 8) ? gates::get_bit(current, i - 8) : 0;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
            // Stage 2: Shift by 4
            if (shamt & 4) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i >= 4) ? gates::get_bit(current, i - 4) : 0;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
            // Stage 1: Shift by 2
            if (shamt & 2) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i >= 2) ? gates::get_bit(current, i - 2) : 0;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
            // Stage 0: Shift by 1
            if (shamt & 1) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i >= 1) ? gates::get_bit(current, i - 1) : 0;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
        } else {
            // Right Shift (LSR or ASR)
            // Stage 4: Shift by 16
            if (shamt & 16) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i + 16 < 32) ? gates::get_bit(current, i + 16) : fillBit;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
            // Stage 3: Shift by 8
            if (shamt & 8) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i + 8 < 32) ? gates::get_bit(current, i + 8) : fillBit;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
            // Stage 2: Shift by 4
            if (shamt & 4) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i + 4 < 32) ? gates::get_bit(current, i + 4) : fillBit;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
            // Stage 1: Shift by 2
            if (shamt & 2) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i + 2 < 32) ? gates::get_bit(current, i + 2) : fillBit;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
            // Stage 0: Shift by 1
            if (shamt & 1) {
                Word next = 0;
                for (int i = 0; i < 32; ++i) {
                    uint8_t bit = (i + 1 < 32) ? gates::get_bit(current, i + 1) : fillBit;
                    gates::set_bit(next, i, bit);
                }
                current = next;
            }
        }

        return current;
    }

    static Word rotateRight(Word val, Word shiftAmount) {
        uint8_t shamt = static_cast<uint8_t>(shiftAmount & 0x1F);
        if (shamt == 0) return val;
        return (val >> shamt) | (val << (32 - shamt));
    }
};

} // namespace alu
} // namespace risc201

#endif // RISC201_SHIFTER_HPP
