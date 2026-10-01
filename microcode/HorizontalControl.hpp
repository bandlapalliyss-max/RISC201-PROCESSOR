#ifndef RISC201_HORIZONTAL_CONTROL_HPP
#define RISC201_HORIZONTAL_CONTROL_HPP

#include "MicroInstructions.hpp"
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

namespace risc201 {
namespace microcode {

/**
 * 65-bit Horizontal Microinstruction Control Word (Sarangi Section 9.8.2, Figure 9.26).
 * Contains:
 *   - 33 control signals bit-vector (directly drives bus multiplexers and registers)
 *   - 12-bit immediate (uimm)
 *   - 10-bit branch target (microPC target)
 *   - 10-bit functional unit args
 */
struct HorizontalControlWord {
    // 33-bit Control Signals
    uint64_t controlSignals{0};

    // Immediate constant (12 bits)
    int16_t uimm{0};

    // Micro-branch target address (10 bits)
    uint16_t ubranchTarget{0};

    // Functional unit arguments (10 bits)
    FunctionalArg args{FunctionalArg::NONE};

    // Human-readable micro-assembly source line for debugger
    std::string disassembly;

    // Bit positions for 33 control signals
    enum ControlBits : uint8_t {
        // Register write-to-bus enable signals (out to write-bus)
        CB_PC_OUT          = 0,
        CB_IR_OUT          = 1,
        CB_I_OUT           = 2,
        CB_RD_OUT          = 3,
        CB_RS1_OUT         = 4,
        CB_RS2_OUT         = 5,
        CB_IMMX_OUT        = 6,
        CB_BRANCH_TGT_OUT  = 7,
        CB_REG_SRC_OUT     = 8,
        CB_REG_DATA_OUT    = 9,
        CB_REG_VAL_OUT     = 10,
        CB_A_OUT           = 11,
        CB_B_OUT           = 12,
        CB_FLAGS_E_OUT     = 13,
        CB_FLAGS_GT_OUT    = 14,
        CB_ALU_RES_OUT     = 15,
        CB_MAR_OUT         = 16,
        CB_MDR_OUT         = 17,
        CB_LD_RES_OUT      = 18,

        // Register read-from-bus enable signals (in from read-bus)
        CB_PC_IN           = 19,
        CB_REG_SRC_IN      = 20,
        CB_REG_DATA_IN     = 21,
        CB_REG_VAL_IN      = 22,
        CB_A_IN            = 23,
        CB_B_IN            = 24,
        CB_ALU_RES_IN      = 25,
        CB_MAR_IN          = 26,
        CB_MDR_IN          = 27,
        CB_LD_RES_IN       = 28,

        // Transfer multiplexer select (2 bits: 0=writeBus, 1=uimm, 2=uadder)
        CB_XFER_MUX_B0     = 29,
        CB_XFER_MUX_B1     = 30,

        // uFetch multiplexer select (2 bits: 0=next_upc, 1=branch_tgt, 2=switch_unit, 3=M1_cond)
        CB_UFETCH_MUX_B0   = 31,
        CB_UFETCH_MUX_B1   = 32
    };

    void setSignal(ControlBits bit, bool val = true) {
        if (val) controlSignals |= (1ULL << bit);
        else controlSignals &= ~(1ULL << bit);
    }

    bool getSignal(ControlBits bit) const {
        return (controlSignals & (1ULL << bit)) != 0;
    }

    uint8_t getXferMux() const {
        return static_cast<uint8_t>((controlSignals >> 29) & 0x3);
    }

    void setXferMux(uint8_t val) {
        controlSignals &= ~(3ULL << 29);
        controlSignals |= (static_cast<uint64_t>(val & 0x3) << 29);
    }

    uint8_t getUfetchMux() const {
        return static_cast<uint8_t>((controlSignals >> 31) & 0x3);
    }

    void setUfetchMux(uint8_t val) {
        controlSignals &= ~(3ULL << 31);
        controlSignals |= (static_cast<uint64_t>(val & 0x3) << 31);
    }
};

} // namespace microcode
} // namespace risc201

#endif // RISC201_HORIZONTAL_CONTROL_HPP
