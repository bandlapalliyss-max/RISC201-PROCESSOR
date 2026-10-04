#include "Debugger.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace risc201;

int main(int argc, char* argv[]) {
    debugger::Debugger dbg;
    std::string programToLoad;

    std::cout << "========================================================================\n"
              << "       RISC201 PROCESSOR SIMULATION & ARCHITECTURE FRAMEWORK            \n"
              << "========================================================================\n";

    // 1. Check if a filename was provided as command-line argument
    if (argc > 1) {
        std::string filename = argv[1];
        std::ifstream file(filename);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            programToLoad = buffer.str();
            std::cout << "Loaded assembly file from command line argument: " << filename << "\n";
        } else {
            std::cerr << "Warning: Could not open file '" << filename << "'.\n";
        }
    }

    // 2. If no valid file argument was given, prompt the user to input the assembly code
    if (programToLoad.empty()) {
        std::cout << "Please provide the RISC201 Assembly language input:\n"
                  << "  * Type/paste your assembly instructions line-by-line.\n"
                  << "  * Type 'END' or 'RUN' on a new line when you are done.\n"
                  << "  * Or enter a file path (e.g. 'benchmarks/factorial_iter.s').\n"
                  << "  * Or press Enter on an empty line to use the built-in demo.\n"
                  << "------------------------------------------------------------------------\n"
                  << "Assembly Input:\n";

        std::string line;
        bool firstLine = true;
        while (std::getline(std::cin, line)) {
            std::string trimmed = assembler::MacroPreprocessor::trim(line);

            // If empty on the first line, fallback to default demonstration program
            if (firstLine && trimmed.empty()) {
                std::cout << "No input provided. Using default recursive factorial demonstration program.\n";
                programToLoad = dbg.getDefaultSampleProgram();
                break;
            }
            firstLine = false;

            std::string upper = trimmed;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            if (upper == "END" || upper == "RUN") {
                break;
            }

            // Check if the user entered an existing filename directly at the prompt
            if (programToLoad.empty() && !trimmed.empty()) {
                std::ifstream testFile(trimmed);
                if (testFile.is_open()) {
                    std::stringstream buf;
                    buf << testFile.rdbuf();
                    programToLoad = buf.str();
                    std::cout << "Loaded assembly from file: " << trimmed << "\n";
                    break;
                }
            }

            programToLoad += line + "\n";
        }

        if (programToLoad.empty()) {
            std::cout << "Loading default demonstration program.\n";
            programToLoad = dbg.getDefaultSampleProgram();
        }
    }

    // 3. Assemble and load the program
    if (dbg.loadAssemblyString(programToLoad)) {
        // 4. Execute the code and display the pipeline execution trace, registers, stack, and stats
        dbg.runWithTrace();
    } else {
        std::cerr << "Assembly failed. Loading default demo program as fallback...\n";
        dbg.loadAssemblyString(dbg.getDefaultSampleProgram());
        dbg.runWithTrace();
    }

    // 5. Enter the interactive CLI Debugger REPL interface (maintains original interface and features)
    dbg.repl();

    return 0;
}
