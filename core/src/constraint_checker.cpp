#include "core/constraint_checker.hpp"

#include <set>

namespace core {

bool ConstraintChecker::canPlace(const Timetable& timetable,
                                 const Subject& subject,
                                 const Faculty& faculty,
                                 const TimeSlot& slot,
                                 std::string& reason) {
    // Rule 1: Faculty cannot teach the subject
    if (!faculty.canTeach(subject.id)) {
        reason = "Faculty " + faculty.name + " cannot teach " + subject.name;
        return false;
    }

    // Rule 2: Faculty is unavailable in that slot
    if (!faculty.isAvailable(slot.id)) {
        reason = "Faculty " + faculty.name + " is unavailable in slot " + slot.toString();
        return false;
    }

    // Rule 3: Slot already used (single class group)
    if (timetable.isSlotUsed(slot.id)) {
        reason = "Slot " + slot.toString() + " is already occupied";
        return false;
    }

    // Rule 4: Faculty already teaching in that slot
    if (timetable.isFacultyBusy(faculty.id, slot.id)) {
        reason = "Faculty " + faculty.name + " is already teaching in slot " + slot.toString();
        return false;
    }

    return true;
}

bool ConstraintChecker::isValid(const Timetable& timetable,
                                const Dataset& dataset,
                                std::vector<std::string>& problems) {
    problems.clear();
    const auto& entries = timetable.getEntries();
    int n = static_cast<int>(entries.size());

    // Check individual entry constraints (Rules 1 & 2 + existence)
    for (int i = 0; i < n; ++i) {
        const auto& e = entries[i];
        const Subject* sub = dataset.findSubject(e.subjectId);
        const Faculty* fac = dataset.findFaculty(e.facultyId);
        const TimeSlot* slot = dataset.findSlot(e.slotId);

        if (!sub) {
            problems.push_back("Unknown subject ID " + std::to_string(e.subjectId));
            continue;
        }
        if (!fac) {
            problems.push_back("Unknown faculty ID " + std::to_string(e.facultyId));
            continue;
        }
        if (!slot) {
            problems.push_back("Unknown slot ID " + std::to_string(e.slotId));
            continue;
        }

        // Rule 1
        if (!fac->canTeach(sub->id)) {
            problems.push_back("Faculty " + fac->name + " cannot teach " + sub->name);
        }

        // Rule 2
        if (!fac->isAvailable(slot->id)) {
            problems.push_back("Faculty " + fac->name + " is unavailable in slot " + slot->toString());
        }
    }

    // Pairwise checks for Rules 3 & 4
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const auto& e1 = entries[i];
            const auto& e2 = entries[j];

            // Rule 3: Slot duplicate
            if (e1.slotId == e2.slotId) {
                const TimeSlot* slot = dataset.findSlot(e1.slotId);
                std::string slotName = slot ? slot->toString() : ("Slot #" + std::to_string(e1.slotId));
                problems.push_back("Slot " + slotName + " is assigned more than once");
            }

            // Rule 4: Faculty duplicate in same slot
            if (e1.facultyId == e2.facultyId && e1.slotId == e2.slotId) {
                const Faculty* fac = dataset.findFaculty(e1.facultyId);
                const TimeSlot* slot = dataset.findSlot(e1.slotId);
                std::string facName = fac ? fac->name : ("Faculty #" + std::to_string(e1.facultyId));
                std::string slotName = slot ? slot->toString() : ("Slot #" + std::to_string(e1.slotId));
                problems.push_back("Faculty " + facName + " is scheduled more than once in " + slotName);
            }
        }
    }

    // Rule 5: Every subject has EXACTLY its periodsPerWeek
    for (const auto& sub : dataset.getSubjects()) {
        int count = timetable.countForSubject(sub.id);
        if (count != sub.periodsPerWeek) {
            problems.push_back("Subject " + sub.name + " has " + std::to_string(count) +
                               " periods scheduled (expected " + std::to_string(sub.periodsPerWeek) + ")");
        }
    }

    return problems.empty();
}

} // namespace core
