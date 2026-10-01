#include "Disassembler.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace risc201;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <input_hex_or_bin_file> [start_pc_hex]\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::ifstream in(inputFile);
    if (!in.is_open()) {
        std::cerr << "Error: Could not open file " << inputFile << "\n";
        return 1;
    }

    Word pc = 0;
    if (argc >= 3) {
        pc = assembler::Disassembler::parseHexString(argv[2]);
    }

    std::cout << "=== Disassembly of " << inputFile << " ===\n";
    std::string line;
    while (std::getline(in, line)) {
        line = assembler::MacroPreprocessor::trim(line);
        if (line.empty() || line[0] == '#') continue;

        Word machineWord = 0;
        if (line.rfind("0x", 0) == 0 || line.rfind("0X", 0) == 0 ||
            (line.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos && line.size() == 8)) {
            machineWord = assembler::Disassembler::parseHexString(line);
        } else if (line.find_first_not_of("01") == std::string::npos && line.size() == 32) {
            machineWord = static_cast<Word>(std::stoul(line, nullptr, 2));
        } else {
            machineWord = assembler::Disassembler::parseHexString(line);
        }

        std::string dis = assembler::Disassembler::disassemble(machineWord, pc);
        std::cout << toHex(pc) << ": " << toHex(machineWord) << "    " << dis << "\n";
        pc += 4;
    }

    return 0;
}
