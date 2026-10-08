#ifndef RISC201_STAGE_REGISTERS_4_HPP
#define RISC201_STAGE_REGISTERS_4_HPP

#include "../common/Types.hpp"
#include <string>

namespace risc201 {
namespace stage4 {
// here we are defining the registers to be used  
struct Latch_IF_OF {
    Word pc{0};
    Word instruction{0};
    bool isBubble{true};
    bool predictedTaken{false};
    Word predictedTarget{0};
    std::string disassembly{"nop"};

    void reset() {
        pc = 0;
        instruction = 0;
        isBubble = true;
        predictedTaken = false;
        predictedTarget = 0;
        disassembly = "nop";
    }
};

struct Latch_OF_EX {
    Word pc{0};
    Word instruction{0};
    DecodedInst decoded{};
    Word op1{0};           
    Word op2{0};          
    Word immx{0};          
    Word branchTarget{0};  
    bool isImmediate{false};
    bool predictedTaken{false};
    Word predictedTarget{0};
    bool isBubble{true};
    std::string disassembly{"nop"};

    void reset() {
        pc = 0;
        instruction = 0;
        decoded = DecodedInst{};
        op1 = op2 = immx = branchTarget = 0;
        isImmediate = false;
        predictedTaken = false;
        predictedTarget = 0;
        isBubble = true;
        disassembly = "nop";
    }
};

struct Latch_EX_MARW {
    Word pc{0};
    Word instruction{0};
    DecodedInst decoded{};
    Word aluResult{0};     
    Word op2{0};           
    bool isBubble{true};
    std::string disassembly{"nop"};

    void reset() {
        pc = 0;
        instruction = 0;
        decoded = DecodedInst{};
        aluResult = op2 = 0;
        isBubble = true;
        disassembly = "nop";
    }
};
} 
} 

#endif 
