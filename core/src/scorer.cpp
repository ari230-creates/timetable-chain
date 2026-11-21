#include "core/scorer.hpp"

#include <cmath>

namespace core {

int Scorer::calculate(const Timetable& timetable, const Dataset& dataset) {
    int score = 0;
    const auto& entries = timetable.getEntries();
    int n = static_cast<int>(entries.size());

    for (int i = 0; i < n; ++i) {
        const auto& e1 = entries[i];
        const TimeSlot* s1 = dataset.findSlot(e1.slotId);
        if (!s1) continue;

        for (int j = i + 1; j < n; ++j) {
            const auto& e2 = entries[j];
            const TimeSlot* s2 = dataset.findSlot(e2.slotId);
            if (!s2) continue;

            // Penalty +5 for each pair of entries with the same subject on the same day
            if (e1.subjectId == e2.subjectId && s1->day == s2->day) {
                score += 5;
            }

            // Penalty +2 for each pair where the same teacher teaches back-to-back periods on the same day
            if (e1.facultyId == e2.facultyId && s1->day == s2->day &&
                std::abs(s1->period - s2->period) == 1) {
                score += 2;
            }
        }
    }

    return score;
}

} // namespace core
