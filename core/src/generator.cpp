#include "core/generator.hpp"
#include "core/constraint_checker.hpp"

#include <algorithm>

namespace core {

Generator::Generator(const Dataset& dataset)
    : m_dataset(dataset) {}

bool Generator::run(GeneratorMode mode,
                    Timetable& outTimetable,
                    const std::atomic<bool>* cancelFlag) {
    m_stats = GeneratorStats();
    m_bestScore = 999999;
    m_bestTimetable = Timetable();

    // Expand each subject into one "task" per required period
    std::vector<Task> tasks;
    for (const auto& s : m_dataset.getSubjects()) {
        for (int p = 0; p < s.periodsPerWeek; ++p) {
            tasks.push_back({s.id});
        }
    }

    if (tasks.empty()) {
        outTimetable.clear();
        return true;
    }

    // For MRV and Best, sort tasks ascending by number of eligible (faculty, slot) options
    if (mode == GeneratorMode::MRV || mode == GeneratorMode::Best) {
        auto countOptions = [&](const Task& t) {
            const auto* sub = m_dataset.findSubject(t.subjectId);
            if (!sub) return 0;
            int count = 0;
            for (const auto& fac : m_dataset.getFaculty()) {
                if (!fac.canTeach(sub->id)) continue;
                for (const auto& slot : m_dataset.getSlots()) {
                    if (fac.isAvailable(slot.id)) {
                        count++;
                    }
                }
            }
            return count;
        };

        std::stable_sort(tasks.begin(), tasks.end(), [&](const Task& a, const Task& b) {
            return countOptions(a) < countOptions(b);
        });
    }

    Timetable current;
    bool foundAny = solve(0, tasks, current, mode, cancelFlag);

    if (mode == GeneratorMode::Best) {
        if (m_stats.solutionsFound > 0) {
            outTimetable = m_bestTimetable;
            m_stats.score = m_bestScore;
            return true;
        }
        return false;
    } else {
        if (foundAny) {
            outTimetable = current;
            m_stats.score = Scorer::calculate(outTimetable, m_dataset);
            return true;
        }
        return false;
    }
}

bool Generator::solve(size_t taskIdx,
                      const std::vector<Task>& tasks,
                      Timetable& current,
                      GeneratorMode mode,
                      const std::atomic<bool>* cancelFlag) {
    if (cancelFlag && cancelFlag->load()) {
        return false;
    }

    // Base case: all tasks placed
    if (taskIdx == tasks.size()) {
        m_stats.solutionsFound++;
        int currentScore = Scorer::calculate(current, m_dataset);
        if (currentScore < m_bestScore) {
            m_bestScore = currentScore;
            m_bestTimetable = current;
        }

        if (mode == GeneratorMode::Best) {
            if (m_stats.solutionsFound >= 50) {
                return true;
            }
            return false; // Keep searching for better solutions
        }

        m_stats.score = currentScore;
        return true;
    }

    const auto& curTask = tasks[taskIdx];
    const auto* sub = m_dataset.findSubject(curTask.subjectId);
    if (!sub) return false;

    // Symmetry breaking: if previous placed entry has the same subject,
    // only consider slots with id strictly greater than its slot id.
    int minSlotId = -1;
    if (!current.getEntries().empty() &&
        current.getEntries().back().subjectId == curTask.subjectId) {
        minSlotId = current.getEntries().back().slotId;
    }

    // Iteration order: faculty outer loop, slot inner loop, both in data-file order
    for (const auto& fac : m_dataset.getFaculty()) {
        for (const auto& slot : m_dataset.getSlots()) {
            if (cancelFlag && cancelFlag->load()) {
                return false;
            }

            if (minSlotId != -1 && slot.id <= minSlotId) {
                continue;
            }

            std::string reason;
            if (!ConstraintChecker::canPlace(current, *sub, fac, slot, reason)) {
                continue;
            }

            // CHOOSE
            current.addEntry({sub->id, fac.id, slot.id});
            m_stats.placements++;

            // EXPLORE
            bool success = solve(taskIdx + 1, tasks, current, mode, cancelFlag);
            if (success) {
                if (mode != GeneratorMode::Best) {
                    return true;
                }
                if (m_stats.solutionsFound >= 50) {
                    return true;
                }
            }

            // UN-CHOOSE
            current.removeLastEntry();
            m_stats.backtracks++;
        }
    }

    return false;
}

} // namespace core
