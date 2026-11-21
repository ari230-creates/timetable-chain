#ifndef TIMETABLE_CHAIN_DATASET_HPP
#define TIMETABLE_CHAIN_DATASET_HPP

#include "core/subject.hpp"
#include "core/faculty.hpp"
#include "core/timeslot.hpp"
#include <vector>
#include <string>

namespace core {

/**
 * @brief Holds all curricular data: subjects, faculty, and available time slots.
 */
class Dataset {
public:
    Dataset() = default;

    /// Clears all loaded data.
    void clear();

    /// Find subject by ID (returns nullptr if not found).
    const Subject* findSubject(int id) const;
    Subject* findSubject(int id);

    /// Find faculty by ID (returns nullptr if not found).
    const Faculty* findFaculty(int id) const;
    Faculty* findFaculty(int id);

    /// Find slot by ID (returns nullptr if not found).
    const TimeSlot* findSlot(int id) const;

    /// Computes the total number of periods needed across all subjects.
    int totalPeriodsNeeded() const;

    /// Getters for domain collections.
    const std::vector<Subject>& getSubjects() const { return m_subjects; }
    const std::vector<Faculty>& getFaculty() const { return m_faculty; }
    const std::vector<TimeSlot>& getSlots() const { return m_slots; }

    /**
     * @brief Loads and parses a data file.
     * 
     * Parses pipe-delimited records:
     *   SUBJECT|id|name|periodsPerWeek
     *   FACULTY|id|name
     *   CAN|facultyId|subjectId
     *   UNAVAILABLE|facultyId|slotId
     *   SLOT|id|day|period
     * 
     * Lines starting with '#' and empty lines are ignored.
     * Errors include line numbers. References are validated after loading.
     * 
     * @param path File system path to the file.
     * @param errorMessage Populated with human-readable error on failure.
     * @return true on success, false on failure.
     */
    bool loadFromFile(const std::string& path, std::string& errorMessage);

    /**
     * @brief Loads and parses curriculum data from a string.
     */
    bool loadFromString(const std::string& content, std::string& errorMessage);

    /**
     * @brief Adds entities directly (useful for tests or programmatic construction).
     */
    void addSubject(const Subject& subject);
    void addFaculty(const Faculty& faculty);
    void addSlot(const TimeSlot& slot);

private:
    std::vector<Subject> m_subjects;
    std::vector<Faculty> m_faculty;
    std::vector<TimeSlot> m_slots;
};

} // namespace core

#endif // TIMETABLE_CHAIN_DATASET_HPP
