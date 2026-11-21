#ifndef TIMETABLE_CHAIN_SCORER_HPP
#define TIMETABLE_CHAIN_SCORER_HPP

#include "core/timetable.hpp"
#include "core/dataset.hpp"

namespace core {

/**
 * @brief Evaluates timetable quality with pedagogical penalty metrics.
 * 
 * Lower penalty score indicates a higher quality schedule:
 *   - Penalty +5 for each pair of entries with the same subject on the same day
 *     (avoids cognitive overload / subject repetition on a single day).
 *   - Penalty +2 for each pair where the same teacher teaches back-to-back periods on the same day
 *     (avoids teacher fatigue without breaks).
 */
class Scorer {
public:
    static int calculate(const Timetable& timetable, const Dataset& dataset);
};

} // namespace core

#endif // TIMETABLE_CHAIN_SCORER_HPP
