#ifndef TIMETABLE_CHAIN_TIMETABLE_HPP
#define TIMETABLE_CHAIN_TIMETABLE_HPP

#include "core/entry.hpp"
#include <vector>
#include <string>

namespace core {

/**
 * @brief Represents a schedule consisting of assigned timetable entries.
 */
class Timetable {
public:
    Timetable() = default;

    /// Adds a new entry to the timetable.
    void addEntry(const Entry& entry);

    /// Removes the most recently added entry (used during backtracking).
    void removeLastEntry();

    /// Removes the entry assigned to the specified time slot ID, if any.
    bool removeEntry(int slotId);

    /// Clears all entries.
    void clear();

    /// Checks if a slot is already occupied.
    bool isSlotUsed(int slotId) const;

    /// Checks if a faculty member is already teaching in the given slot.
    bool isFacultyBusy(int facultyId, int slotId) const;

    /// Counts how many times a subject has been scheduled.
    int countForSubject(int subjectId) const;

    /// Returns a const reference to the current list of entries.
    const std::vector<Entry>& getEntries() const { return m_entries; }

    /// Returns entry at the specified slot ID, or nullptr if unoccupied.
    const Entry* findEntryBySlot(int slotId) const;

    /**
     * @brief Serializes the timetable to a deterministic string.
     * 
     * Entries are sorted by (slotId, subjectId, facultyId) so that identical
     * schedules produce identical serialized representations for hashing.
     * Output format: "subj,fac,slot;subj,fac,slot;..."
     */
    std::string serialize() const;

    /**
     * @brief Deserializes a formatted string back into a Timetable.
     */
    static Timetable deserialize(const std::string& str);

private:
    std::vector<Entry> m_entries;
};

} // namespace core

#endif // TIMETABLE_CHAIN_TIMETABLE_HPP
