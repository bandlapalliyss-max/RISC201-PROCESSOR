#include "Assembler.hpp"
#include <fstream>
#include <iostream>

using namespace risc201;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <input.s> [output.hex] [output.bin]\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::ifstream in(inputFile);
    if (!in.is_open()) {
        std::cerr << "Error: Could not open input file " << inputFile << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();

    assembler::Assembler asmb;
    try {
        asmb.assemble(source);
    } catch (const std::exception& e) {
        std::cerr << "Assembly Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "=== Assembly Listing for " << inputFile << " ===\n";
    std::cout << asmb.getListing();

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
