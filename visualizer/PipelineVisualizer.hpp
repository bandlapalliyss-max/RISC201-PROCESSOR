#ifndef RISC201_PIPELINE_VISUALIZER_HPP
#define RISC201_PIPELINE_VISUALIZER_HPP

#include "../common/Types.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace risc201 {
namespace visualizer {

struct PipelineStageInfo {
    std::string stageName;
    std::string instructionText;
    Word pc{0};
    bool isBubble{false};
    bool isStalled{false};
};

/**
 * Pipeline Visualizer rendering cycle-by-cycle pipeline diagrams
 * and stage latch state in ASCII art format (Sarangi Section 10.4.1).
 */
class PipelineVisualizer {
public:
    static std::string renderStages(const std::vector<PipelineStageInfo>& stages, uint64_t cycle) {
        std::ostringstream oss;
        oss << "\n+--------------------------------------------------------------------------+\n";
        oss << "|               PIPELINE EXECUTION STATE (Clock Cycle: " << std::setw(6) << cycle << ")             |\n";
        oss << "+--------------------------------------------------------------------------+\n";

        for (const auto& st : stages) {
            oss << "  [" << std::setw(6) << std::left << st.stageName << "] : ";
            if (st.isBubble) {
                oss << "-- BUBBLE / NOP --";
            } else if (st.isStalled) {
                oss << "[STALLED] " << st.instructionText << " (PC: " << toHex(st.pc) << ")";
            } else if (!st.instructionText.empty()) {
                oss << st.instructionText << " (PC: " << toHex(st.pc) << ")";
            } else {
                oss << "(empty)";
            }
            oss << "\n";
        }
        oss << "+--------------------------------------------------------------------------+\n";
        return oss.str();
    }
};

} // namespace visualizer
} // namespace risc201

#endif // RISC201_PIPELINE_VISUALIZER_HPP
