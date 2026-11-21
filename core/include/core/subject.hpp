#ifndef TIMETABLE_CHAIN_SUBJECT_HPP
#define TIMETABLE_CHAIN_SUBJECT_HPP

#include <string>

namespace core {

/**
 * @brief Represents an academic subject course.
 */
struct Subject {
    int id = 0;
    std::string name;
    int periodsPerWeek = 0;

    Subject() = default;
    Subject(int id_, std::string name_, int periods_)
        : id(id_), name(std::move(name_)), periodsPerWeek(periods_) {}
};

} // namespace core

#endif // TIMETABLE_CHAIN_SUBJECT_HPP
