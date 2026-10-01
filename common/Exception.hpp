#ifndef RISC201_EXCEPTION_HPP
#define RISC201_EXCEPTION_HPP

#include "Types.hpp"
#include <string>
#include <exception>

namespace risc201 {

// Runtime Exception Types as specified in the assignment
enum class ExceptionType {
    NONE = 0,
    STACK_OVERFLOW,          // Stack pointer sp decremented below stack limit
    STACK_UNDERFLOW,         // Stack pointer sp incremented above stack base
    STACK_CORRUPTION,        // Invalid memory write/access into stack guard region
    UNALIGNED_MEMORY_ACCESS, // Memory word access not 4-byte aligned
    OUT_OF_BOUNDS_MEMORY,    // Access outside allocated memory space
    DIVISION_BY_ZERO,        // div or mod by zero
    ILLEGAL_OPCODE,          // Unknown instruction opcode
    PRIVILEGE_VIOLATION      // User mode attempting privileged operation (e.g. movz, retz)
};

inline std::string exceptionToString(ExceptionType type) {
    switch (type) {
        case ExceptionType::NONE: return "No Exception";
        case ExceptionType::STACK_OVERFLOW: return "Stack Overflow Exception (sp < STACK_LIMIT)";
        case ExceptionType::STACK_UNDERFLOW: return "Stack Underflow Exception (sp > STACK_BASE)";
        case ExceptionType::STACK_CORRUPTION: return "Stack Boundary Corruption Detected";
        case ExceptionType::UNALIGNED_MEMORY_ACCESS: return "Unaligned Memory Access Exception (not multiple of 4)";
        case ExceptionType::OUT_OF_BOUNDS_MEMORY: return "Out-of-Bounds Memory Exception";
        case ExceptionType::DIVISION_BY_ZERO: return "Division/Modulo by Zero Exception";
        case ExceptionType::ILLEGAL_OPCODE: return "Illegal Opcode Exception";
        case ExceptionType::PRIVILEGE_VIOLATION: return "Privilege Violation Exception (CPL=1 attempting CPL=0)";
        default: return "Unknown Exception";
    }
}

// Exception record containing fault context
struct CpuException {
    ExceptionType type{ExceptionType::NONE};
    Word faultingPC{0};
    Word faultingAddress{0};
    std::string details;

    bool hasOccurred() const { return type != ExceptionType::NONE; }
    void clear() {
        type = ExceptionType::NONE;
        faultingPC = 0;
        faultingAddress = 0;
        details.clear();
    }
};

// Stack Boundary Configurator & Verifier
struct StackBounds {
    Word stackBase{0x000FFFFC};   // Highest address where stack begins (grows downward)
    Word stackLimit{0x00080000};  // Lowest allowed stack pointer address (stack size = 512KB)
    Word guardBandSize{64};       // 64-byte guard zone below stack limit to catch corruption

    // Verify if sp is within bounds
    ExceptionType verifySp(Word sp) const {
        if (sp > stackBase) {
            return ExceptionType::STACK_UNDERFLOW;
        }
        if (sp < stackLimit) {
            return ExceptionType::STACK_OVERFLOW;
        }
        return ExceptionType::NONE;
    }

    // Verify if memory write corrupts stack boundaries or guard band
    ExceptionType verifyWriteAddress(Word addr) const {
        if (addr >= stackLimit - guardBandSize && addr < stackLimit) {
            return ExceptionType::STACK_CORRUPTION;
        }
        return ExceptionType::NONE;
    }
};

} // namespace risc201

#endif // RISC201_EXCEPTION_HPP
