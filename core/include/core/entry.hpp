#ifndef TIMETABLE_CHAIN_ENTRY_HPP
#define TIMETABLE_CHAIN_ENTRY_HPP

namespace core {

/**
 * @brief An assigned timetable entry binding subject, faculty, and time slot.
 */
struct Entry {
    int subjectId = 0;
    int facultyId = 0;
    int slotId = 0;

    bool operator==(const Entry& other) const {
        return subjectId == other.subjectId &&
               facultyId == other.facultyId &&
               slotId == other.slotId;
    }

    bool operator!=(const Entry& other) const {
        return !(*this == other);
    }
};

} // namespace core

#endif // TIMETABLE_CHAIN_ENTRY_HPP
