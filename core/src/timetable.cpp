#include "core/timetable.hpp"

#include <algorithm>
#include <sstream>

namespace core {

void Timetable::addEntry(const Entry& entry) {
    m_entries.push_back(entry);
}

void Timetable::removeLastEntry() {
    if (!m_entries.empty()) {
        m_entries.pop_back();
    }
}

bool Timetable::removeEntry(int slotId) {
    auto it = std::find_if(m_entries.begin(), m_entries.end(), [slotId](const Entry& e) {
        return e.slotId == slotId;
    });
    if (it != m_entries.end()) {
        m_entries.erase(it);
        return true;
    }
    return false;
}

void Timetable::clear() {
    m_entries.clear();
}

bool Timetable::isSlotUsed(int slotId) const {
    for (const auto& e : m_entries) {
        if (e.slotId == slotId) return true;
    }
    return false;
}

bool Timetable::isFacultyBusy(int facultyId, int slotId) const {
    for (const auto& e : m_entries) {
        if (e.facultyId == facultyId && e.slotId == slotId) return true;
    }
    return false;
}

int Timetable::countForSubject(int subjectId) const {
    int count = 0;
    for (const auto& e : m_entries) {
        if (e.subjectId == subjectId) {
            count++;
        }
    }
    return count;
}

const Entry* Timetable::findEntryBySlot(int slotId) const {
    for (const auto& e : m_entries) {
        if (e.slotId == slotId) return &e;
    }
    return nullptr;
}

std::string Timetable::serialize() const {
    std::vector<Entry> sorted = m_entries;
    std::sort(sorted.begin(), sorted.end(), [](const Entry& a, const Entry& b) {
        if (a.slotId != b.slotId) return a.slotId < b.slotId;
        if (a.subjectId != b.subjectId) return a.subjectId < b.subjectId;
        return a.facultyId < b.facultyId;
    });

    std::string result;
    for (const auto& e : sorted) {
        result += std::to_string(e.subjectId) + "," +
                  std::to_string(e.facultyId) + "," +
                  std::to_string(e.slotId) + ";";
    }
    return result;
}

Timetable Timetable::deserialize(const std::string& str) {
    Timetable tt;
    if (str.empty()) return tt;

    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ';')) {
        if (token.empty()) continue;
        std::stringstream tokenStream(token);
        std::string sSubj, sFac, sSlot;
        if (std::getline(tokenStream, sSubj, ',') &&
            std::getline(tokenStream, sFac, ',') &&
            std::getline(tokenStream, sSlot, ',')) {
            try {
                int subjId = std::stoi(sSubj);
                int facId = std::stoi(sFac);
                int slotId = std::stoi(sSlot);
                tt.addEntry({subjId, facId, slotId});
            } catch (...) {
                // Ignore malformed token
            }
        }
    }
    return tt;
}

} // namespace core
