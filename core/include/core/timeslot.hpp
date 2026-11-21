#ifndef TIMETABLE_CHAIN_TIMESLOT_HPP
#define TIMETABLE_CHAIN_TIMESLOT_HPP

#include <string>

namespace core {

/**
 * @brief Represents a single period in the weekly schedule.
 */
struct TimeSlot {
    int id = 0;
    std::string day;    // e.g. "Mon", "Tue", "Wed"
    int period = 0;     // 1-based index, e.g. 1, 2, 3

    TimeSlot() = default;
    TimeSlot(int id_, std::string day_, int period_)
        : id(id_), day(std::move(day_)), period(period_) {}

    /**
     * @brief Human-readable string representation, e.g. "Mon P1".
     */
    std::string toString() const {
        return day + " P" + std::to_string(period);
    }
};

} // namespace core

#endif // TIMETABLE_CHAIN_TIMESLOT_HPP
