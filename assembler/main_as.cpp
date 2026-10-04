#include "Assembler.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace risc201;

int main(int argc, char* argv[]) {
    std::string source;
    std::string inputFile = "stdin";

    if (argc >= 2) {
        inputFile = argv[1];
        std::ifstream in(inputFile);
        if (!in.is_open()) {
            std::cerr << "Error: Could not open input file " << inputFile << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << in.rdbuf();
        source = buffer.str();
    } else {
        std::cout << "========================================================================\n"
                  << "                 RISC201 TWO-PASS ASSEMBLER (Pass 1 & 2)                \n"
                  << "========================================================================\n"
                  << "Enter RISC201 Assembly code (Type code lines, then 'END' on a new line;\n"
                  << "or enter a file path e.g. 'benchmarks/factorial_iter.s'):\n"
                  << "------------------------------------------------------------------------\n";
        std::string line;
        while (std::getline(std::cin, line)) {
            std::string trimmed = assembler::MacroPreprocessor::trim(line);
            std::string upper = trimmed;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            if (upper == "END") break;
            if (source.empty() && !trimmed.empty()) {
                std::ifstream testFile(trimmed);
                if (testFile.is_open()) {
                    std::stringstream buf;
                    buf << testFile.rdbuf();
                    source = buf.str();
                    inputFile = trimmed;
                    std::cout << "Loaded assembly from file: " << trimmed << "\n";
                    break;
                }
            }
            source += line + "\n";
        }
    }

    if (assembler::MacroPreprocessor::trim(source).empty()) {
        std::cerr << "No assembly input provided.\n";
        return 1;
    }

    assembler::Assembler asmb;
    try {
        asmb.assemble(source);
    } catch (const std::exception& e) {
        std::cerr << "Assembly Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "=== Assembly Listing for " << inputFile << " ===\n";
    std::cout << asmb.getListing();

    if (!asmb.symbolTable.empty()) {
        std::cout << "\n=== Symbol Table ===\n";
        for (const auto& kv : asmb.symbolTable) {
            std::cout << "  " << std::left << std::setw(20) << kv.first << " : " << toHex(kv.second) << "\n";
        }
        std::cout << "\n";
    }

    if (argc >= 3) {
        std::string hexFile = argv[2];
        std::ofstream outHex(hexFile);
        for (const auto& line : asmb.assembledLines) {
            outHex << line.hexString << "\n";
        }
        std::cout << "Wrote hexadecimal machine code to " << hexFile << "\n";
    }

    if (argc >= 4) {
        std::string binFile = argv[3];
        std::ofstream outBin(binFile);
        for (const auto& line : asmb.assembledLines) {
            outBin << line.binaryString << "\n";
        }
        std::cout << "Wrote binary bit-strings to " << binFile << "\n";
    }

    return 0;
}
