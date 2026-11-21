#ifndef TIMETABLE_CHAIN_CONSTRAINT_CHECKER_HPP
#define TIMETABLE_CHAIN_CONSTRAINT_CHECKER_HPP

#include "core/timetable.hpp"
#include "core/dataset.hpp"
#include <string>
#include <vector>

namespace core {

/**
 * @brief Enforces curricular hard constraints and schedule validity.
 */
class ConstraintChecker {
public:
    /**
     * @brief Checks if a specific placement is valid for the current timetable state.
     * 
     * Verifies rules:
     *   1. Faculty can teach the subject.
     *   2. Faculty is available in the time slot.
     *   3. Time slot is not already occupied by another subject.
     *   4. Faculty is not already teaching in that slot.
     * 
     * @param timetable Current partial or full timetable.
     * @param subject Subject to place.
     * @param faculty Faculty assigned to teach.
     * @param slot Target time slot.
     * @param reason Populated with human-readable rejection reason if invalid.
     * @return true if valid placement, false otherwise.
     */
    static bool canPlace(const Timetable& timetable,
                         const Subject& subject,
                         const Faculty& faculty,
                         const TimeSlot& slot,
                         std::string& reason);

    /**
     * @brief Performs complete verification of an entire timetable.
     * 
     * Checks:
     *   - Rules 1-4 for all entries pairwise and individually.
     *   - Rule 5: Every subject has EXACTLY its required periodsPerWeek.
     * 
     * @param timetable The timetable to validate.
     * @param dataset Reference curricular dataset.
     * @param problems Populated with descriptive error messages for every violation.
     * @return true if completely valid, false if any violation exists.
     */
    static bool isValid(const Timetable& timetable,
                        const Dataset& dataset,
                        std::vector<std::string>& problems);
};

} // namespace core

#endif // TIMETABLE_CHAIN_CONSTRAINT_CHECKER_HPP
