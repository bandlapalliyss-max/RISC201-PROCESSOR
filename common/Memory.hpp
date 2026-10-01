#ifndef RISC201_MEMORY_HPP
#define RISC201_MEMORY_HPP

#include "Types.hpp"
#include "Exception.hpp"
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace risc201 {

/**
 * Memory implements byte-addressable linear physical memory for RISC201.
 * Supports Little-Endian multi-byte access (Sarangi Section 3.3.2).
 * Integrates stack boundary protection and alignment safety verification.
 */
class Memory {
private:
    std::vector<Byte> mem;
    StackBounds stackBounds;
    size_t sizeBytes;

public:
    explicit Memory(size_t size = 1024 * 1024) // Default 1MB
        : mem(size, 0), sizeBytes(size) {}

    void reset() {
        std::fill(mem.begin(), mem.end(), 0);
    }

    size_t getSize() const { return sizeBytes; }
    const StackBounds& getStackBounds() const { return stackBounds; }
    void setStackBounds(const StackBounds& bounds) { stackBounds = bounds; }

    // Read 1 byte
    Byte readByte(Word addr, CpuException* ex = nullptr) const {
        if (addr >= sizeBytes) {
            if (ex) {
                ex->type = ExceptionType::OUT_OF_BOUNDS_MEMORY;
                ex->faultingAddress = addr;
                ex->details = "Memory read out of bounds: " + toHex(addr);
            }
            return 0;
        }
        return mem[addr];
    }

    // Write 1 byte
    void writeByte(Word addr, Byte val, CpuException* ex = nullptr) {
        if (addr >= sizeBytes) {
            if (ex) {
                ex->type = ExceptionType::OUT_OF_BOUNDS_MEMORY;
                ex->faultingAddress = addr;
                ex->details = "Memory write out of bounds: " + toHex(addr);
            }
            return;
        }
        // Verify stack boundary protection
        ExceptionType stackEx = stackBounds.verifyWriteAddress(addr);
        if (stackEx != ExceptionType::NONE) {
            if (ex) {
                ex->type = stackEx;
                ex->faultingAddress = addr;
                ex->details = "Write into stack guard band: " + toHex(addr);
            }
            return;
        }
        mem[addr] = val;
    }

    // Read 32-bit Word (Little-Endian)
    Word readWord(Word addr, CpuException* ex = nullptr) const {
        // Alignment verification: word addresses must be aligned to 4 bytes
        if ((addr & 0x3) != 0) {
            if (ex) {
                ex->type = ExceptionType::UNALIGNED_MEMORY_ACCESS;
                ex->faultingAddress = addr;
                ex->details = "Unaligned word read at: " + toHex(addr);
            }
            return 0;
        }
        if (addr + 3 >= sizeBytes) {
            if (ex) {
                ex->type = ExceptionType::OUT_OF_BOUNDS_MEMORY;
                ex->faultingAddress = addr;
                ex->details = "Word read out of bounds at: " + toHex(addr);
            }
            return 0;
        }
        // Little-endian reconstruction: byte 0 is LSB, byte 3 is MSB
        Word b0 = mem[addr + 0];
        Word b1 = mem[addr + 1];
        Word b2 = mem[addr + 2];
        Word b3 = mem[addr + 3];
        return (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }

    // Write 32-bit Word (Little-Endian)
    void writeWord(Word addr, Word val, CpuException* ex = nullptr) {
        // Alignment verification
        if ((addr & 0x3) != 0) {
            if (ex) {
                ex->type = ExceptionType::UNALIGNED_MEMORY_ACCESS;
                ex->faultingAddress = addr;
                ex->details = "Unaligned word write at: " + toHex(addr);
            }
            return;
        }
        if (addr + 3 >= sizeBytes) {
            if (ex) {
                ex->type = ExceptionType::OUT_OF_BOUNDS_MEMORY;
                ex->faultingAddress = addr;
                ex->details = "Word write out of bounds at: " + toHex(addr);
            }
            return;
        }
        // Check stack guard corruption
        ExceptionType stackEx = stackBounds.verifyWriteAddress(addr);
        if (stackEx != ExceptionType::NONE) {
            if (ex) {
                ex->type = stackEx;
                ex->faultingAddress = addr;
                ex->details = "Write into stack guard band at: " + toHex(addr);
            }
            return;
        }
        // Little-endian storage
        mem[addr + 0] = static_cast<Byte>(val & 0xFF);
        mem[addr + 1] = static_cast<Byte>((val >> 8) & 0xFF);
        mem[addr + 2] = static_cast<Byte>((val >> 16) & 0xFF);
        mem[addr + 3] = static_cast<Byte>((val >> 24) & 0xFF);
    }

    // Load program words at a specified base address
    void loadProgram(Word baseAddr, const std::vector<Word>& words) {
        for (size_t i = 0; i < words.size(); ++i) {
            writeWord(baseAddr + static_cast<Word>(i * 4), words[i]);
        }
    }

    // Hex dump for inspection in CLI debugger
    std::string dump(Word startAddr, Word countWords) const {
        std::ostringstream oss;
        Word alignedStart = startAddr & ~0x3;
        for (Word i = 0; i < countWords; ++i) {
            Word curr = alignedStart + i * 4;
            if (curr + 3 < sizeBytes) {
                Word val = (static_cast<Word>(mem[curr])) |
                           (static_cast<Word>(mem[curr + 1]) << 8) |
                           (static_cast<Word>(mem[curr + 2]) << 16) |
                           (static_cast<Word>(mem[curr + 3]) << 24);
                oss << toHex(curr) << ": " << toHex(val) << "\n";
            }
        }
        return oss.str();
    }
};

} // namespace risc201

#endif // RISC201_MEMORY_HPP
