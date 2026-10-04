CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2 -I.

TARGETS = risc201_sim risc201_as risc201_disasm risc201_eval

all: $(TARGETS)

risc201_sim: debugger/main_sim.cpp common/*.hpp alu/*.hpp microcode/*.hpp assembler/*.hpp visualizer/*.hpp processor_4stage/*.hpp processor_6stage/*.hpp debugger/*.hpp
	$(CXX) $(CXXFLAGS) debugger/main_sim.cpp -o risc201_sim

risc201_as: assembler/main_as.cpp common/*.hpp assembler/*.hpp
	$(CXX) $(CXXFLAGS) assembler/main_as.cpp -o risc201_as

risc201_disasm: assembler/main_disasm.cpp common/*.hpp assembler/*.hpp
	$(CXX) $(CXXFLAGS) assembler/main_disasm.cpp -o risc201_disasm

risc201_eval: evaluation/CompareProcessors.cpp common/*.hpp alu/*.hpp microcode/*.hpp assembler/*.hpp processor_4stage/*.hpp processor_6stage/*.hpp
	$(CXX) $(CXXFLAGS) evaluation/CompareProcessors.cpp -o risc201_eval

clean:
	rm -f $(TARGETS) *.exe *.hex *.bin

.PHONY: all clean
