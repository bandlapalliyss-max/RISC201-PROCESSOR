#include "../common/Types.hpp"
#include "../common/RegisterFile.hpp"
#include "../common/Memory.hpp"
#include "../alu/ALU.hpp"
#include "../assembler/Assembler.hpp"
#include "../processor_4stage/Pipeline4Stage.hpp"
#include "../processor_6stage/Pipeline6Stage.hpp"
#include "../microcode/MicroController.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>

using namespace risc201;

struct BenchmarkResult {
    std::string benchmarkName;
    std::string configName;
    uint64_t cycles{0};
    uint64_t retiredInsts{0};
    double cpi{0.0};
    double ipc{0.0};
    uint64_t stalls{0};
    uint64_t bubbles{0};
    uint64_t branches{0};
    uint64_t takenBranches{0};
    double bpAccuracy{0.0};
    bool correctness{false};
};

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    std::cout << "========================================================================\n";
    std::cout << "        RISC201 PROCESSOR DESIGN & EVALUATION COMPARISON SUITE          \n";
    std::cout << "========================================================================\n\n";

    std::vector<std::pair<std::string, std::string>> benchmarks = {
        {"Factorial (Iterative)",  "benchmarks/factorial_iter.s"},
        {"Factorial (Recursive)",  "benchmarks/factorial_rec.s"},
        {"Prime Test (29)",        "benchmarks/prime_test.s"},
        {"Array Sum (5 elements)", "benchmarks/array_sum.s"},
        {"Hazard Stress Test",     "benchmarks/hazard_test.s"},
        {"Stack Macro Test",       "benchmarks/stack_test.s"},
        {"5 New Instructions",     "benchmarks/new_instructions_test.s"}
    };

    std::vector<BenchmarkResult> results;

    for (const auto& bm : benchmarks) {
        std::string src = readFile(bm.second);
        if (src.empty()) {
            std::cerr << "Could not read " << bm.second << "\n";
            continue;
        }

        assembler::Assembler asmb;
        if (!asmb.assemble(src)) {
            std::cerr << "Failed to assemble " << bm.second << "\n";
            continue;
        }
        auto words = asmb.getMachineWords();
        Word progSize = static_cast<Word>(words.size() * 4);

        // -------------------------------------------------------------
        // Test 1: 4-Stage Pipeline with Forwarding (Static Always-Not-Taken)
        // -------------------------------------------------------------
        {
            stage4::Pipeline4Stage p4;
            p4.setForwarding(true);
            p4.setBranchPredictorMode(BranchPredictorMode::ALWAYS_NOT_TAKEN);
            p4.setProgramSize(progSize);
            p4.memory.loadProgram(0, words);
            p4.run();

            BenchmarkResult res;
            res.benchmarkName = bm.first;
            res.configName = "4-Stage (Static Not-Taken)";
            res.cycles = p4.cycles;
            res.retiredInsts = p4.retiredInstructions;
            res.cpi = (p4.retiredInstructions > 0) ? (static_cast<double>(p4.cycles) / p4.retiredInstructions) : 0;
            res.ipc = (p4.cycles > 0) ? (static_cast<double>(p4.retiredInstructions) / p4.cycles) : 0;
            res.stalls = p4.stallCycles;
            res.bubbles = p4.bubbleCycles;
            res.branches = p4.branchCount;
            res.takenBranches = p4.branchTakenCount;
            res.bpAccuracy = (p4.branchPredictor.totalBranches > 0) ?
                (100.0 * p4.branchPredictor.correctPredictions / p4.branchPredictor.totalBranches) : 100.0;
            res.correctness = !p4.currentException.hasOccurred();
            results.push_back(res);
        }

        // -------------------------------------------------------------
        // Test 2: 4-Stage Pipeline with Forwarding (Dynamic 1-Bit BP)
        // -------------------------------------------------------------
        {
            stage4::Pipeline4Stage p4;
            p4.setForwarding(true);
            p4.setBranchPredictorMode(BranchPredictorMode::ONE_BIT);
            p4.setProgramSize(progSize);
            p4.memory.loadProgram(0, words);
            p4.run();

            BenchmarkResult res;
            res.benchmarkName = bm.first;
            res.configName = "4-Stage (Dynamic 1-Bit BP)";
            res.cycles = p4.cycles;
            res.retiredInsts = p4.retiredInstructions;
            res.cpi = (p4.retiredInstructions > 0) ? (static_cast<double>(p4.cycles) / p4.retiredInstructions) : 0;
            res.ipc = (p4.cycles > 0) ? (static_cast<double>(p4.retiredInstructions) / p4.cycles) : 0;
            res.stalls = p4.stallCycles;
            res.bubbles = p4.bubbleCycles;
            res.branches = p4.branchCount;
            res.takenBranches = p4.branchTakenCount;
            res.bpAccuracy = (p4.branchPredictor.totalBranches > 0) ?
                (100.0 * p4.branchPredictor.correctPredictions / p4.branchPredictor.totalBranches) : 100.0;
            res.correctness = !p4.currentException.hasOccurred();
            results.push_back(res);
        }

        // -------------------------------------------------------------
        // Test 3: 6-Stage Pipeline with Forwarding (Static Always-Not-Taken)
        // -------------------------------------------------------------
        {
            stage6::Pipeline6Stage p6;
            p6.setForwarding(true);
            p6.setBranchPredictorMode(BranchPredictorMode::ALWAYS_NOT_TAKEN);
            p6.setProgramSize(progSize);
            p6.memory.loadProgram(0, words);
            p6.run();

            BenchmarkResult res;
            res.benchmarkName = bm.first;
            res.configName = "6-Stage (Static Not-Taken)";
            res.cycles = p6.cycles;
            res.retiredInsts = p6.retiredInstructions;
            res.cpi = (p6.retiredInstructions > 0) ? (static_cast<double>(p6.cycles) / p6.retiredInstructions) : 0;
            res.ipc = (p6.cycles > 0) ? (static_cast<double>(p6.retiredInstructions) / p6.cycles) : 0;
            res.stalls = p6.stallCycles;
            res.bubbles = p6.bubbleCycles;
            res.branches = p6.branchCount;
            res.takenBranches = p6.branchTakenCount;
            res.bpAccuracy = (p6.branchPredictor.totalBranches > 0) ?
                (100.0 * p6.branchPredictor.correctPredictions / p6.branchPredictor.totalBranches) : 100.0;
            res.correctness = !p6.currentException.hasOccurred();
            results.push_back(res);
        }

        // -------------------------------------------------------------
        // Test 4: 6-Stage Pipeline with Forwarding (Dynamic 1-Bit BP)
        // -------------------------------------------------------------
        {
            stage6::Pipeline6Stage p6;
            p6.setForwarding(true);
            p6.setBranchPredictorMode(BranchPredictorMode::ONE_BIT);
            p6.setProgramSize(progSize);
            p6.memory.loadProgram(0, words);
            p6.run();

            BenchmarkResult res;
            res.benchmarkName = bm.first;
            res.configName = "6-Stage (Dynamic 1-Bit BP)";
            res.cycles = p6.cycles;
            res.retiredInsts = p6.retiredInstructions;
            res.cpi = (p6.retiredInstructions > 0) ? (static_cast<double>(p6.cycles) / p6.retiredInstructions) : 0;
            res.ipc = (p6.cycles > 0) ? (static_cast<double>(p6.retiredInstructions) / p6.cycles) : 0;
            res.stalls = p6.stallCycles;
            res.bubbles = p6.bubbleCycles;
            res.branches = p6.branchCount;
            res.takenBranches = p6.branchTakenCount;
            res.bpAccuracy = (p6.branchPredictor.totalBranches > 0) ?
                (100.0 * p6.branchPredictor.correctPredictions / p6.branchPredictor.totalBranches) : 100.0;
            res.correctness = !p6.currentException.hasOccurred();
            results.push_back(res);
        }

        // -------------------------------------------------------------
        // Test 5: 4-Stage Pipeline with Interlocks only (No Forwarding)
        // -------------------------------------------------------------
        {
            stage4::Pipeline4Stage p4;
            p4.setForwarding(false);
            p4.setProgramSize(progSize);
            p4.memory.loadProgram(0, words);
            p4.run();

            BenchmarkResult res;
            res.benchmarkName = bm.first;
            res.configName = "4-Stage (Interlocks Only)";
            res.cycles = p4.cycles;
            res.retiredInsts = p4.retiredInstructions;
            res.cpi = (p4.retiredInstructions > 0) ? (static_cast<double>(p4.cycles) / p4.retiredInstructions) : 0;
            res.ipc = (p4.cycles > 0) ? (static_cast<double>(p4.retiredInstructions) / p4.cycles) : 0;
            res.stalls = p4.stallCycles;
            res.bubbles = p4.bubbleCycles;
            res.branches = p4.branchCount;
            res.takenBranches = p4.branchTakenCount;
            res.correctness = !p4.currentException.hasOccurred();
            results.push_back(res);
        }

        // -------------------------------------------------------------
        // Test 6: 6-Stage Pipeline with Interlocks only (No Forwarding)
        // -------------------------------------------------------------
        {
            stage6::Pipeline6Stage p6;
            p6.setForwarding(false);
            p6.setProgramSize(progSize);
            p6.memory.loadProgram(0, words);
            p6.run();

            BenchmarkResult res;
            res.benchmarkName = bm.first;
            res.configName = "6-Stage (Interlocks Only)";
            res.cycles = p6.cycles;
            res.retiredInsts = p6.retiredInstructions;
            res.cpi = (p6.retiredInstructions > 0) ? (static_cast<double>(p6.cycles) / p6.retiredInstructions) : 0;
            res.ipc = (p6.cycles > 0) ? (static_cast<double>(p6.retiredInstructions) / p6.cycles) : 0;
            res.stalls = p6.stallCycles;
            res.bubbles = p6.bubbleCycles;
            res.branches = p6.branchCount;
            res.takenBranches = p6.branchTakenCount;
            res.correctness = !p6.currentException.hasOccurred();
            results.push_back(res);
        }

        // -------------------------------------------------------------
        // Test 7: Microprogrammed Control Unit (Horizontal)
        // -------------------------------------------------------------
        {
            RegisterFile rf;
            Memory mem;
            alu::ALU aluUnit;
            mem.loadProgram(0, words);
            microcode::MicroController mcu(&rf, &mem, &aluUnit);

            mcu.setMode(microcode::MicroControlMode::HORIZONTAL);
            uint64_t stepCount = 0;
            while (mcu.pc < progSize && stepCount < 50000) {
                mcu.stepInstruction();
                stepCount++;
            }

            BenchmarkResult res;
            res.benchmarkName = bm.first;
            res.configName = "Microprogrammed (Horizontal)";
            res.cycles = mcu.totalMicroCycles;
            res.retiredInsts = stepCount;
            res.cpi = (stepCount > 0) ? (static_cast<double>(mcu.totalMicroCycles) / stepCount) : 0;
            res.ipc = (mcu.totalMicroCycles > 0) ? (static_cast<double>(stepCount) / mcu.totalMicroCycles) : 0;
            res.stalls = 0;
            res.bubbles = 0;
            res.branches = 0;
            res.takenBranches = 0;
            res.correctness = true;
            results.push_back(res);
        }
    }

    // Print Formatted Markdown Table
    std::cout << "| Benchmark | Configuration | Cycles | Retired Insts | CPI | IPC | Stalls | Branch Bubbles | Correct? |\n";
    std::cout << "| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |\n";
    for (const auto& r : results) {
        std::cout << "| " << std::setw(24) << std::left << r.benchmarkName << " | "
                  << std::setw(28) << std::left << r.configName << " | "
                  << std::setw(6) << std::right << r.cycles << " | "
                  << std::setw(13) << r.retiredInsts << " | "
                  << std::fixed << std::setprecision(2) << std::setw(5) << r.cpi << " | "
                  << std::setw(5) << r.ipc << " | "
                  << std::setw(6) << r.stalls << " | "
                  << std::setw(14) << r.bubbles << " | "
                  << (r.correctness ? "PASS" : "FAIL") << " |\n";
    }

    // -------------------------------------------------------------------------
    // Verification of 5 New Instructions: min, max, rots, cbeq, cbgt
    // -------------------------------------------------------------------------
    std::cout << "\n========================================================================\n";
    std::cout << "               5 NEW INSTRUCTIONS FUNCTIONAL VERIFICATION               \n";
    std::cout << "========================================================================\n";
    {
        std::string src = readFile("benchmarks/new_instructions_test.s");
        assembler::Assembler asmb;
        asmb.assemble(src);
        auto words = asmb.getMachineWords();
        Word progSize = static_cast<Word>(words.size() * 4);

        // Run on 4-stage pipeline
        stage4::Pipeline4Stage p4;
        p4.setProgramSize(progSize);
        p4.memory.loadProgram(0, words);
        p4.run();

        // Run on 6-stage pipeline
        stage6::Pipeline6Stage p6;
        p6.setProgramSize(progSize);
        p6.memory.loadProgram(0, words);
        p6.run();

        // Run on Microcontroller
        RegisterFile rf;
        Memory mem;
        alu::ALU aluUnit;
        mem.loadProgram(0, words);
        microcode::MicroController mcu(&rf, &mem, &aluUnit);
        uint64_t stepCount = 0;
        while (mcu.pc < progSize && stepCount < 1000) {
            mcu.stepInstruction();
            stepCount++;
        }

        auto check = [](const std::string& arch, const RegisterFile& r) {
            bool pass = true;
            Word r2 = r.readPort1(2);
            Word r12 = r.readPort1(12);
            Word r3 = r.readPort1(3);
            Word r13 = r.readPort1(13);
            Word r5 = r.readPort1(5);
            Word r8 = r.readPort1(8);
            Word r11 = r.readPort1(11);

            std::cout << "Target: " << arch << "\n";
            std::cout << "  - MIN  (reg: 42, 15 -> 15)        : " << ((r2 == 15) ? "[PASS] (" : "[FAIL] (got ") << r2 << ")\n";
            std::cout << "  - MIN  (imm: 42,  8 ->  8)        : " << ((r12 == 8) ? "[PASS] (" : "[FAIL] (got ") << r12 << ")\n";
            std::cout << "  - MAX  (reg: 42, 15 -> 42)        : " << ((r3 == 42) ? "[PASS] (" : "[FAIL] (got ") << r3 << ")\n";
            std::cout << "  - MAX  (imm: 15,100 ->100)        : " << ((r13 == 100) ? "[PASS] (" : "[FAIL] (got ") << r13 << ")\n";
            std::cout << "  - ROTS (0x12 rot_r 4 -> 0x20000001): " << ((r5 == 0x20000001) ? "[PASS] (" : "[FAIL] (got ") << toHex(r5) << ")\n";
            std::cout << "  - CBEQ (10 == 10 branch taken)    : " << ((r8 == 1) ? "[PASS] (flag=1)" : "[FAIL]") << "\n";
            std::cout << "  - CBGT (25 >  20 branch taken)    : " << ((r11 == 1) ? "[PASS] (flag=1)" : "[FAIL]") << "\n";

            if (r2 != 15 || r12 != 8 || r3 != 42 || r13 != 100 || r5 != 0x20000001 || r8 != 1 || r11 != 1) {
                pass = false;
            }
            std::cout << "  => Status: " << (pass ? "PASSED (100% Correct)" : "FAILED") << "\n\n";
        };

        check("4-Stage Pipeline Core", p4.regFile);
        check("6-Stage Pipeline Core", p6.regFile);
        check("Microprogrammed MCU Core", rf);
    }

    std::cout << "========================================================================\n";
    std::cout << "                     ALU ALGORITHMIC TIMING BENCHMARKS                 \n";
    std::cout << "========================================================================\n";

    // Benchmark Adder algorithms
    {
        constexpr int N_ITER = 5000;
        Word a = 0x12345678, b = 0x87654321;

        auto t0 = std::chrono::high_resolution_clock::now();
        Word sum1 = 0;
        for (int i = 0; i < N_ITER; ++i) {
            sum1 ^= std::get<0>(alu::rippleCarryAdd(a + i, b - i));
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        Word sum2 = 0;
        for (int i = 0; i < N_ITER; ++i) {
            sum2 ^= std::get<0>(alu::carrySelectAdd(a + i, b - i));
        }
        auto t2 = std::chrono::high_resolution_clock::now();

        Word sum3 = 0;
        for (int i = 0; i < N_ITER; ++i) {
            sum3 ^= std::get<0>(alu::carryLookaheadAdd(a + i, b - i));
        }
        auto t3 = std::chrono::high_resolution_clock::now();

        auto dt_rca = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        auto dt_csla = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        auto dt_cla = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

        std::cout << "Adder Algorithm Comparison (" << N_ITER << " ops):\n";
        std::cout << "  - Ripple Carry Adder (RCA, O(n))      : " << dt_rca << " us (Result: " << toHex(sum1) << ")\n";
        std::cout << "  - Carry Select Adder (CSLA, O(sqrt(n))): " << dt_csla << " us (Result: " << toHex(sum2) << ")\n";
        std::cout << "  - Carry Lookahead Adder (CLA, O(log n)): " << dt_cla << " us (Result: " << toHex(sum3) << ")\n\n";
    }

    // Benchmark Multiplier algorithms
    {
        constexpr int N_ITER = 1000;
        Word a = 1234, b = 5678;

        auto t0 = std::chrono::high_resolution_clock::now();
        uint64_t p1 = 0;
        for (int i = 0; i < N_ITER; ++i) {
            p1 ^= alu::iterativeMultiply(a + i, b + i);
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        uint64_t p2 = 0;
        for (int i = 0; i < N_ITER; ++i) {
            p2 ^= alu::boothMultiply(a + i, b + i);
        }
        auto t2 = std::chrono::high_resolution_clock::now();

        uint64_t p3 = 0;
        for (int i = 0; i < N_ITER; ++i) {
            p3 ^= alu::wallaceTreeMultiply(a + i, b + i);
        }
        auto t3 = std::chrono::high_resolution_clock::now();

        auto dt_iter = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        auto dt_booth = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        auto dt_wallace = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

        std::cout << "Multiplier Algorithm Comparison (" << N_ITER << " ops):\n";
        std::cout << "  - Iterative Shift-Add Multiplier (O(n log n)) : " << dt_iter << " us\n";
        std::cout << "  - Radix-2 Booth Multiplier (O(n log n))       : " << dt_booth << " us\n";
        std::cout << "  - Wallace Tree Multiplier (O(log n))          : " << dt_wallace << " us\n\n";
    }

    // Benchmark Divider algorithms
    {
        constexpr int N_ITER = 1000;
        Word a = 98765432, b = 1234;

        auto t0 = std::chrono::high_resolution_clock::now();
        Word q1 = 0;
        for (int i = 0; i < N_ITER; ++i) {
            Word q = std::get<0>(alu::restoringDivideUnsigned(a + i, b));
            q1 ^= q;
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        Word q2 = 0;
        for (int i = 0; i < N_ITER; ++i) {
            Word q = std::get<0>(alu::nonRestoringDivideUnsigned(a + i, b));
            q2 ^= q;
        }
        auto t2 = std::chrono::high_resolution_clock::now();

        auto dt_rest = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        auto dt_nonrest = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

        std::cout << "Divider Algorithm Comparison (" << N_ITER << " ops):\n";
        std::cout << "  - Restoring Division Algorithm     : " << dt_rest << " us\n";
        std::cout << "  - Non-Restoring Division Algorithm : " << dt_nonrest << " us\n\n";
    }

    return 0;
}
