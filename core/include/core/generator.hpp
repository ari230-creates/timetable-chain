#ifndef TIMETABLE_CHAIN_GENERATOR_HPP
#define TIMETABLE_CHAIN_GENERATOR_HPP

#include "core/timetable.hpp"
#include "core/dataset.hpp"
#include "core/scorer.hpp"
#include <vector>
#include <atomic>
#include <functional>

namespace core {

/**
 * @brief Search strategy modes for timetable generation.
 */
enum class GeneratorMode {
    Plain,  ///< Tasks in data-file subject order; returns first valid timetable.
    MRV,    ///< Minimum Remaining Values: most constrained tasks first.
    Best    ///< MRV search exploring up to 50 solutions, keeping the lowest penalty score.
};

/**
 * @brief Execution metrics produced by the search algorithm.
 */
struct GeneratorStats {
    int placements = 0;       ///< Number of assignment attempts made
    int backtracks = 0;       ///< Number of times an assignment was undone
    int solutionsFound = 0;   ///< Number of complete valid timetables found
    int score = 0;            ///< Quality score of selected timetable (lower is better)
};

/**
 * @brief DAA Recursive Backtracking Constraint Satisfaction Solver.
 */
class Generator {
public:
    explicit Generator(const Dataset& dataset);

    /**
     * @brief Executes the timetable generation algorithm.
     * 
     * @param mode Search strategy (Plain, MRV, or Best).
     * @param outTimetable Receives the resulting schedule on success.
     * @param cancelFlag Optional pointer to atomic boolean to signal early stop.
     * @return true if a valid timetable was found, false otherwise.
     */
    bool run(GeneratorMode mode,
             Timetable& outTimetable,
             const std::atomic<bool>* cancelFlag = nullptr);

    /// Access statistics from the most recent run.
    const GeneratorStats& getStats() const { return m_stats; }

private:
    struct Task {
        int subjectId;
    };

    bool solve(size_t taskIdx,
               const std::vector<Task>& tasks,
               Timetable& current,
               GeneratorMode mode,
               const std::atomic<bool>* cancelFlag);

    const Dataset& m_dataset;
    GeneratorStats m_stats;
    Timetable m_bestTimetable;
    int m_bestScore = 999999;
};

} // namespace core

#endif // TIMETABLE_CHAIN_GENERATOR_HPP
