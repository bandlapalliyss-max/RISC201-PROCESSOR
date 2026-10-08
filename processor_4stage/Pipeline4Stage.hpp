#ifndef RISC201_PIPELINE_4STAGE_HPP
#define RISC201_PIPELINE_4STAGE_HPP

#include "StageRegisters4.hpp"
#include "../common/Types.hpp"
#include "../common/RegisterFile.hpp"
#include "../common/Memory.hpp"
#include "../common/Exception.hpp"
#include "../common/BranchPredictor.hpp"
#include "../alu/ALU.hpp"
#include "../assembler/Disassembler.hpp"
#include "../visualizer/PipelineVisualizer.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace risc201 {
namespace stage4 {

class Pipeline4Stage {
public:
    RegisterFile regFile;
    Memory memory;
    alu::ALU aluUnit;
    BranchPredictor branchPredictor;
    Word pc{0};
    Word programSize{0};
    bool halted{false};
    CpuException currentException{};

    // two latches were used so that one stage will not see the value produced in the other stage in the same cycle 
    Latch_IF_OF l_IF_OF{};
    Latch_OF_EX l_OF_EX{};
    Latch_EX_MARW l_EX_MARW{};

    Latch_IF_OF next_IF_OF{};
    Latch_OF_EX next_OF_EX{};
    Latch_EX_MARW next_EX_MARW{};

    // terms that are to be shown at end after each these are updated 
    uint64_t cycles{0};
    uint64_t retiredInstructions{0};
    uint64_t stallCycles{0};
    uint64_t bubbleCycles{0};
    uint64_t branchCount{0};
    uint64_t branchTakenCount{0};

    bool forwardingEnabled{true};

public:
    explicit Pipeline4Stage(size_t memSize = 1024 * 1024)
        : memory(memSize) {
        reset();
    }

    //if we have entered reset button the complete vallues comes to the initial stage.
    void reset() {
        regFile.reset();
        memory.reset();
        branchPredictor.reset();
        pc = 0;
        programSize = 0;
        halted = false;
        currentException.clear();

        l_IF_OF.reset();
        l_OF_EX.reset();
        l_EX_MARW.reset();
        next_IF_OF.reset();
        next_OF_EX.reset();
        next_EX_MARW.reset();

        cycles = 0;
        retiredInstructions = 0;
        stallCycles = 0;
        bubbleCycles = 0;
        branchCount = 0;
        branchTakenCount = 0;
    }
    // here we will be selecting the size and setting whether the forwarding is enabled or not and which type of predictor is being used.
    void setProgramSize(Word sz) { programSize = sz; }
    void setForwarding(bool enable) { forwardingEnabled = enable; }
    bool isForwardingEnabled() const { return forwardingEnabled; }
    void setBranchPredictorMode(BranchPredictorMode m) { branchPredictor.setMode(m); }
    BranchPredictorMode getBranchPredictorMode() const { return branchPredictor.getMode(); }

    // check whether in all stages there is no instruction so that we can end the cycles
    bool isDrained() const {
        return (programSize > 0 && pc >= programSize) &&
               l_IF_OF.isBubble && l_OF_EX.isBubble && l_EX_MARW.isBubble;
    }
    // executing one cycle 
    void stepCycle() {
        if (halted) return;
        cycles++;
        
        bool stall_IF = false;
        bool stall_OF = false;
        bool bubble_EX = false;
        bool isBranchTaken = false;
        Word branchPC = 0;
        
        //stage 4 MA
        if (!l_EX_MARW.isBubble) {
            const auto& d = l_EX_MARW.decoded;
            Word writebackData = l_EX_MARW.aluResult;

            if (d.isLoad()) {
                // Read word from memory
                writebackData = memory.readWord(l_EX_MARW.aluResult, &currentException);
            } else if (d.isStore()) {
                // Store op2 into memory at aluResult
                memory.writeWord(l_EX_MARW.aluResult, l_EX_MARW.op2, &currentException);
            } else if (d.isCall()) {
                // Call writes return address (PC + 4)
                writebackData = l_EX_MARW.pc + 4;
            }

            // Writeback to register file if instruction produces a register value
            if (d.writesRegister() && !currentException.hasOccurred()) {
                regFile.writePort(d.getDestReg(), writebackData, true);
            }

            retiredInstructions++;
        }
        //EX stage execution 
        if (!l_OF_EX.isBubble) {
            const auto& d = l_OF_EX.decoded; 
            Word opA = l_OF_EX.op1;
            if (forwardingEnabled && !l_EX_MARW.isBubble && l_EX_MARW.decoded.writesRegister()) {
                RegId destPrev = l_EX_MARW.decoded.getDestReg();
                if (d.readsRs1() && d.rs1 == destPrev) {
                    if (l_EX_MARW.decoded.isLoad()) {
                        opA = memory.readWord(l_EX_MARW.aluResult, nullptr);
                    } else if (l_EX_MARW.decoded.isCall()) {
                        opA = l_EX_MARW.pc + 4;
                    } else {
                        opA = l_EX_MARW.aluResult;
                    }
                }
            }
            Word opB = l_OF_EX.isImmediate ? l_OF_EX.immx : l_OF_EX.op2;
            if (!l_OF_EX.isImmediate && forwardingEnabled && !l_EX_MARW.isBubble && l_EX_MARW.decoded.writesRegister()) {
                RegId destPrev = l_EX_MARW.decoded.getDestReg();
                if (d.readsRs2() && d.rs2 == destPrev) {
                    if (l_EX_MARW.decoded.isLoad()) {
                        opB = memory.readWord(l_EX_MARW.aluResult, nullptr);
                    } else if (l_EX_MARW.decoded.isCall()) {
                        opB = l_EX_MARW.pc + 4;
                    } else {
                        opB = l_EX_MARW.aluResult;
                    }
                }
            }
            Word storeData = l_OF_EX.op2;
            if (d.isStore() && forwardingEnabled && !l_EX_MARW.isBubble && l_EX_MARW.decoded.writesRegister()) {
                if (d.rd == l_EX_MARW.decoded.getDestReg()) {
                    if (l_EX_MARW.decoded.isLoad()) {
                        storeData = memory.readWord(l_EX_MARW.aluResult, nullptr);
                    } else {
                        storeData = l_EX_MARW.aluResult;
                    }
                }
            }

            auto aluOut = aluUnit.execute(d.opcode, opA, opB, regFile.getFlags());
            if (d.opcode == OP_CMP) {
                regFile.setFlags(aluOut.flags);
            }
            if (aluOut.exception.hasOccurred()) {
                currentException = aluOut.exception;
                currentException.faultingPC = l_OF_EX.pc;
            }

            // Branch Condition Evaluation
            if (d.isBranch()) {
                branchCount++;
                Flags flg = regFile.getFlags();

                if (d.opcode == OP_B) {
                    isBranchTaken = true;
                    branchPC = l_OF_EX.branchTarget;
                } else if (d.opcode == OP_BEQ) {
                    isBranchTaken = flg.E;
                    branchPC = l_OF_EX.branchTarget;
                } else if (d.opcode == OP_BGT) {
                    isBranchTaken = flg.GT;
                    branchPC = l_OF_EX.branchTarget;
                } else if (d.opcode == OP_CBEQ) {
                    isBranchTaken = (opA == opB);
                    branchPC = l_OF_EX.branchTarget;
                } else if (d.opcode == OP_CBGT) {
                    isBranchTaken = (static_cast<int32_t>(opA) > static_cast<int32_t>(opB));
                    branchPC = l_OF_EX.branchTarget;
                } else if (d.opcode == OP_CALL) {
                    isBranchTaken = true;
                    branchPC = l_OF_EX.branchTarget;
                } else if (d.opcode == OP_RET) {
                    isBranchTaken = true;
                    branchPC = opA; // ra
                } else if (d.opcode == OP_RETZ) {
                    isBranchTaken = true;
                    branchPC = regFile.getOldPC();
                    regFile.setCPL(1);
                }

                if (isBranchTaken) {
                    branchTakenCount++;
                }

                branchPredictor.update(l_OF_EX.pc, isBranchTaken, branchPC, l_OF_EX.predictedTaken);
            }
            // writing the next instructions after EX stage 
            next_EX_MARW.pc = l_OF_EX.pc;
            next_EX_MARW.instruction = l_OF_EX.instruction;
            next_EX_MARW.decoded = d;
            next_EX_MARW.aluResult = aluOut.result;
            next_EX_MARW.op2 = storeData;
            next_EX_MARW.isBubble = false;
            next_EX_MARW.disassembly = l_OF_EX.disassembly;
        } else {
            next_EX_MARW.reset();
        }

        if (!l_IF_OF.isBubble) {
            DecodedInst d = assembler::Disassembler::decode(l_IF_OF.instruction, l_IF_OF.pc);

            // checking for hazaards here 
            bool loadUseHazard = false;
            if (!l_OF_EX.isBubble && l_OF_EX.decoded.isLoad()) {
                RegId loadDest = l_OF_EX.decoded.rd;
                if ((d.readsRs1() && d.rs1 == loadDest) ||
                    (d.readsRs2() && d.rs2 == loadDest) ||
                    (d.isStore() && d.rd == loadDest)) {
                    loadUseHazard = true;
                }
            }
            // checking for raw hazards ther or not 
            bool rawHazardNoForward = false;
            if (!forwardingEnabled) {
                if (!l_OF_EX.isBubble && l_OF_EX.decoded.writesRegister()) {
                    RegId dest = l_OF_EX.decoded.getDestReg();
                    if ((d.readsRs1() && d.rs1 == dest) ||
                        (d.readsRs2() && d.rs2 == dest) ||
                        (d.isStore() && d.rd == dest)) {
                        rawHazardNoForward = true;
                    }
                }
            }
            //if hazards are there then add stalls 
            if (loadUseHazard || rawHazardNoForward) {
                stall_IF = true;
                stall_OF = true;
                bubble_EX = true;
                stallCycles++;
            } else {
                // Reading register operands 
                Word op1 = regFile.readPort1(d.isRet() ? REG_RA : d.rs1);
                Word op2 = regFile.readPort2(d.isStore() ? d.rd : d.rs2);
                
                if (forwardingEnabled && !l_EX_MARW.isBubble && l_EX_MARW.decoded.writesRegister()) {
                    RegId wbReg = l_EX_MARW.decoded.getDestReg();
                    Word wbData = l_EX_MARW.aluResult;
                    if (l_EX_MARW.decoded.isLoad()) {
                        wbData = memory.readWord(l_EX_MARW.aluResult, nullptr);
                    } else if (l_EX_MARW.decoded.isCall()) {
                        wbData = l_EX_MARW.pc + 4;
                    }
                    if (d.readsRs1() && d.rs1 == wbReg) op1 = wbData;
                    if (d.readsRs2() && d.rs2 == wbReg) op2 = wbData;
                    if (d.isStore() && d.rd == wbReg) op2 = wbData;
                }
                // updating the values 
                next_OF_EX.pc = l_IF_OF.pc;
                next_OF_EX.instruction = l_IF_OF.instruction;
                next_OF_EX.decoded = d;
                next_OF_EX.op1 = op1;
                next_OF_EX.op2 = op2;
                next_OF_EX.immx = d.immx;
                next_OF_EX.branchTarget = d.branchTarget;
                next_OF_EX.isImmediate = d.isImmediate;
                next_OF_EX.predictedTaken = l_IF_OF.predictedTaken;
                next_OF_EX.predictedTarget = l_IF_OF.predictedTarget;
                next_OF_EX.isBubble = false;
                next_OF_EX.disassembly = l_IF_OF.disassembly;
            }
        } else {
            next_OF_EX.reset();
        }
        //checking whether mis prediction happened or not 
        bool mispredicted = false;
        if (!l_OF_EX.isBubble && l_OF_EX.decoded.isBranch()) {
            if (isBranchTaken != l_OF_EX.predictedTaken) {
                mispredicted = true;
            } else if (isBranchTaken && branchPC != l_OF_EX.predictedTarget) {
                mispredicted = true;
            }
        }

        // is misprediction took place we are adding jump/flush 
        if (mispredicted) {
            pc = isBranchTaken ? branchPC : (l_OF_EX.pc + 4);
            next_IF_OF.reset(); // Flushing IP
            next_OF_EX.reset();  //flushing OF
            bubbleCycles += 2;
        } else if (programSize > 0 && pc >= programSize) {
            next_IF_OF.reset(); 
        } else if (!stall_IF) {
            Word instWord = memory.readWord(pc, &currentException);
            if (currentException.hasOccurred()) {
                currentException.faultingPC = pc;
                halted = true;
                return;
            }
                                 
            Word fetchPC = pc;
            auto pred = branchPredictor.predict(fetchPC);
            // again updating the fetch values 
            next_IF_OF.pc = fetchPC;
            next_IF_OF.instruction = instWord;
            next_IF_OF.isBubble = false;
            next_IF_OF.predictedTaken = pred.predictedTaken;
            next_IF_OF.predictedTarget = pred.predictedTarget;
            next_IF_OF.disassembly = assembler::Disassembler::disassemble(instWord, fetchPC);

            if (pred.predictedTaken) {
                pc = pred.predictedTarget;
            } else {
                pc += 4;
            }
        }
        
        if (bubble_EX) {
            l_OF_EX.reset();
        } else if (!stall_OF) {
            l_OF_EX = next_OF_EX;
        }

        if (!stall_IF) {
            l_IF_OF = next_IF_OF;
        }
        l_EX_MARW = next_EX_MARW;
    }

    // if we type run then this makes the pipeline structure for us untill the pipeline is drained or untilll it reaches the max cycles 
    void run(uint64_t maxCycles = 100000) {
        while (!halted && cycles < maxCycles) {
            stepCycle();
            if (isDrained()) {
                break;
            }
        }
    }

    std::vector<visualizer::PipelineStageInfo> getStageInfo() const {
        std::vector<visualizer::PipelineStageInfo> info;
        info.push_back({"IF", l_IF_OF.isBubble ? "" : l_IF_OF.disassembly, l_IF_OF.pc, l_IF_OF.isBubble, false});
        info.push_back({"OF", l_OF_EX.isBubble ? "" : l_OF_EX.disassembly, l_OF_EX.pc, l_OF_EX.isBubble, false});
        info.push_back({"EX", l_EX_MARW.isBubble ? "" : l_EX_MARW.disassembly, l_EX_MARW.pc, l_EX_MARW.isBubble, false});
        info.push_back({"MA_RW", l_EX_MARW.isBubble ? "" : l_EX_MARW.disassembly, l_EX_MARW.pc, l_EX_MARW.isBubble, false});
        return info;
    }

    std::string printStats() const {
        std::ostringstream oss;
        double cpi = (retiredInstructions > 0) ? (static_cast<double>(cycles) / retiredInstructions) : 0.0;
        double ipc = (cycles > 0) ? (static_cast<double>(retiredInstructions) / cycles) : 0.0;

        oss << "=== 4-Stage Pipeline Statistics ===\n"
            << "Total Clock Cycles      : " << cycles << "\n"
            << "Retired Instructions    : " << retiredInstructions << "\n"
            << "Cycles Per Inst (CPI)   : " << std::fixed << std::setprecision(3) << cpi << "\n"
            << "Inst Per Cycle (IPC)    : " << std::fixed << std::setprecision(3) << ipc << "\n"
            << "Stall Cycles (RAW/Load) : " << stallCycles << "\n"
            << "Branch Bubble Cycles    : " << bubbleCycles << "\n"
            << "Total Branches          : " << branchCount << "\n"
            << "Taken Branches          : " << branchTakenCount << "\n"
            << "Forwarding Enabled      : " << (forwardingEnabled ? "YES" : "NO (Interlocks only)") << "\n"
            << branchPredictor.printStats();
        return oss.str();
    }
};

} // namespace stage4
} // namespace risc201

#endif // RISC201_PIPELINE_4STAGE_HPP
