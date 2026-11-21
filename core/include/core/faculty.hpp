#ifndef TIMETABLE_CHAIN_FACULTY_HPP
#define TIMETABLE_CHAIN_FACULTY_HPP

#include <string>
#include <set>

namespace core {

/**
 * @brief Represents a faculty member (instructor / teacher).
 */
struct Faculty {
    int id = 0;
    std::string name;
    std::set<int> canTeachSubjects; // Set of subject IDs this faculty can teach
    std::set<int> unavailableSlots; // Set of slot IDs where this faculty is not available

    Faculty() = default;
    Faculty(int id_, std::string name_)
        : id(id_), name(std::move(name_)) {}

    /**
     * @brief Checks if this faculty is qualified to teach the specified subject.
     */
    bool canTeach(int subjectId) const {
        return canTeachSubjects.find(subjectId) != canTeachSubjects.end();
    }

    /**
     * @brief Checks if this faculty is available during the specified time slot.
     */
    bool isAvailable(int slotId) const {
        return unavailableSlots.find(slotId) == unavailableSlots.end();
    }
};

} // namespace core

#endif // TIMETABLE_CHAIN_FACULTY_HPP
