#ifndef RISC201_BRANCH_PREDICTOR_HPP
#define RISC201_BRANCH_PREDICTOR_HPP

#include "Types.hpp"
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

namespace risc201 {

/**
 * Branch Prediction Modes (Idea 3: Lightweight Branch Prediction):
 *   - ALWAYS_NOT_TAKEN: Baseline static prediction (predict fall-through PC+4).
 *   - ONE_BIT: Dynamic 1-bit Branch History Table (BHT) with Branch Target Buffer (BTB).
 */
enum class BranchPredictorMode {
    ALWAYS_NOT_TAKEN,
    ONE_BIT
};

struct PredictionResult {
    bool predictedTaken{false};
    Word predictedTarget{0};
};

/**
 * Lightweight Branch Predictor Class.
 * Evaluates performance difference between static Always-Not-Taken and 1-bit dynamic predictor.
 */
class BranchPredictor {
public:
    static constexpr size_t BHT_SIZE = 128; // 128-entry table
    BranchPredictorMode mode{BranchPredictorMode::ALWAYS_NOT_TAKEN};

    std::vector<uint8_t> bht;   // 1-bit prediction state (0 = Not-Taken, 1 = Taken)
    std::vector<Word> btb;      // Cached target branch address
    std::vector<bool> btbValid; // BTB entry valid bit

    // Performance metrics
    uint64_t totalBranches{0};
    uint64_t correctPredictions{0};
    uint64_t mispredictions{0};

public:
    explicit BranchPredictor(BranchPredictorMode initialMode = BranchPredictorMode::ALWAYS_NOT_TAKEN)
        : mode(initialMode), bht(BHT_SIZE, 0), btb(BHT_SIZE, 0), btbValid(BHT_SIZE, false) {}

    void reset() {
        std::fill(bht.begin(), bht.end(), 0);
        std::fill(btb.begin(), btb.end(), 0);
        std::fill(btbValid.begin(), btbValid.end(), false);
        totalBranches = 0;
        correctPredictions = 0;
        mispredictions = 0;
    }

    void setMode(BranchPredictorMode m) {
        mode = m;
        reset();
    }

    BranchPredictorMode getMode() const { return mode; }

    std::string getModeName() const {
        return (mode == BranchPredictorMode::ALWAYS_NOT_TAKEN) ?
               "ALWAYS_NOT_TAKEN (Static Baseline)" : "ONE_BIT (Dynamic 1-Bit Predictor)";
    }

    size_t getIndex(Word pc) const {
        return (pc >> 2) & (BHT_SIZE - 1);
    }

    PredictionResult predict(Word pc) const {
        PredictionResult res;
        if (mode == BranchPredictorMode::ALWAYS_NOT_TAKEN) {
            res.predictedTaken = false;
            res.predictedTarget = pc + 4;
            return res;
        }

        size_t idx = getIndex(pc);
        if (bht[idx] == 1 && btbValid[idx]) {
            res.predictedTaken = true;
            res.predictedTarget = btb[idx];
        } else {
            res.predictedTaken = false;
            res.predictedTarget = pc + 4;
        }
        return res;
    }

    void update(Word pc, bool actualTaken, Word actualTarget, bool wasPredictedTaken) {
        totalBranches++;
        bool isCorrect = (actualTaken == wasPredictedTaken);
        if (isCorrect) {
            correctPredictions++;
        } else {
            mispredictions++;
        }

        if (mode == BranchPredictorMode::ONE_BIT) {
            size_t idx = getIndex(pc);
            bht[idx] = actualTaken ? 1 : 0;
            if (actualTaken) {
                btb[idx] = actualTarget;
                btbValid[idx] = true;
            }
        }
    }

    std::string printStats() const {
        std::ostringstream oss;
        double accuracy = (totalBranches > 0) ?
            (100.0 * static_cast<double>(correctPredictions) / totalBranches) : 100.0;
        oss << "Branch Predictor Mode   : " << getModeName() << "\n"
            << "Total Conditional Br's  : " << totalBranches << "\n"
            << "Correct Predictions     : " << correctPredictions << "\n"
            << "Mispredictions          : " << mispredictions << "\n"
            << "Prediction Accuracy     : " << std::fixed << std::setprecision(1) << accuracy << "%\n";
        return oss.str();
    }
};

} // namespace risc201

#endif // RISC201_BRANCH_PREDICTOR_HPP
