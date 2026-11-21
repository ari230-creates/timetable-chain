#include "core/dataset.hpp"

#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>

namespace core {

namespace {

// Helper to trim leading/trailing whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Helper to split a line by delimiter
std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delim)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

} // anonymous namespace

void Dataset::clear() {
    m_subjects.clear();
    m_faculty.clear();
    m_slots.clear();
}

const Subject* Dataset::findSubject(int id) const {
    for (const auto& s : m_subjects) {
        if (s.id == id) return &s;
    }
    return nullptr;
}

Subject* Dataset::findSubject(int id) {
    for (auto& s : m_subjects) {
        if (s.id == id) return &s;
    }
    return nullptr;
}

const Faculty* Dataset::findFaculty(int id) const {
    for (const auto& f : m_faculty) {
        if (f.id == id) return &f;
    }
    return nullptr;
}

Faculty* Dataset::findFaculty(int id) {
    for (auto& f : m_faculty) {
        if (f.id == id) return &f;
    }
    return nullptr;
}

const TimeSlot* Dataset::findSlot(int id) const {
    for (const auto& s : m_slots) {
        if (s.id == id) return &s;
    }
    return nullptr;
}

int Dataset::totalPeriodsNeeded() const {
    int total = 0;
    for (const auto& s : m_subjects) {
        total += s.periodsPerWeek;
    }
    return total;
}

void Dataset::addSubject(const Subject& subject) {
    m_subjects.push_back(subject);
}

void Dataset::addFaculty(const Faculty& faculty) {
    m_faculty.push_back(faculty);
}

void Dataset::addSlot(const TimeSlot& slot) {
    m_slots.push_back(slot);
}

bool Dataset::loadFromFile(const std::string& path, std::string& errorMessage) {
    std::ifstream file(path);
    if (!file.is_open()) {
        errorMessage = "Unable to open file: " + path;
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str(), errorMessage);
}

bool Dataset::loadFromString(const std::string& content, std::string& errorMessage) {
    std::istringstream stream(content);

    std::vector<Subject> newSubjects;
    std::vector<Faculty> newFaculty;
    std::vector<TimeSlot> newSlots;
    std::vector<std::pair<int, int>> canTeachList;
    std::vector<std::pair<int, int>> unavailableList;

    std::set<int> subjectIds;
    std::set<int> facultyIds;
    std::set<int> slotIds;

    std::string line;
    int lineNumber = 0;

    while (std::getline(stream, line)) {
        lineNumber++;
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }

        std::vector<std::string> parts = split(trimmed, '|');
        if (parts.empty()) continue;

        const std::string& type = parts[0];

        try {
            if (type == "SUBJECT") {
                if (parts.size() < 4) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Invalid SUBJECT format. Expected SUBJECT|id|name|periodsPerWeek";
                    return false;
                }
                int id = std::stoi(parts[1]);
                const std::string& name = parts[2];
                int periods = std::stoi(parts[3]);
                if (periods < 0) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Negative periodsPerWeek for subject ID " + std::to_string(id);
                    return false;
                }
                if (!subjectIds.insert(id).second) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Duplicate subject ID " + std::to_string(id);
                    return false;
                }
                newSubjects.emplace_back(id, name, periods);
            } else if (type == "FACULTY") {
                if (parts.size() < 3) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Invalid FACULTY format. Expected FACULTY|id|name";
                    return false;
                }
                int id = std::stoi(parts[1]);
                const std::string& name = parts[2];
                if (!facultyIds.insert(id).second) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Duplicate faculty ID " + std::to_string(id);
                    return false;
                }
                newFaculty.emplace_back(id, name);
            } else if (type == "CAN") {
                if (parts.size() < 3) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Invalid CAN format. Expected CAN|facultyId|subjectId";
                    return false;
                }
                int facId = std::stoi(parts[1]);
                int subId = std::stoi(parts[2]);
                canTeachList.emplace_back(facId, subId);
            } else if (type == "UNAVAILABLE") {
                if (parts.size() < 3) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Invalid UNAVAILABLE format. Expected UNAVAILABLE|facultyId|slotId";
                    return false;
                }
                int facId = std::stoi(parts[1]);
                int slotId = std::stoi(parts[2]);
                unavailableList.emplace_back(facId, slotId);
            } else if (type == "SLOT") {
                if (parts.size() < 4) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Invalid SLOT format. Expected SLOT|id|day|period";
                    return false;
                }
                int id = std::stoi(parts[1]);
                const std::string& day = parts[2];
                int period = std::stoi(parts[3]);
                if (!slotIds.insert(id).second) {
                    errorMessage = "Line " + std::to_string(lineNumber) + ": Duplicate slot ID " + std::to_string(id);
                    return false;
                }
                newSlots.emplace_back(id, day, period);
            } else {
                errorMessage = "Line " + std::to_string(lineNumber) + ": Unknown record type '" + type + "'";
                return false;
            }
        } catch (const std::exception& ex) {
            errorMessage = "Line " + std::to_string(lineNumber) + ": Parse error (" + ex.what() + ")";
            return false;
        }
    }

    // Referential integrity validation after entire file is read
    for (const auto& item : canTeachList) {
        int facId = item.first;
        int subId = item.second;
        if (facultyIds.find(facId) == facultyIds.end()) {
            errorMessage = "Referential integrity error: CAN references non-existent faculty ID " + std::to_string(facId);
            return false;
        }
        if (subjectIds.find(subId) == subjectIds.end()) {
            errorMessage = "Referential integrity error: CAN references non-existent subject ID " + std::to_string(subId);
            return false;
        }
        for (auto& f : newFaculty) {
            if (f.id == facId) {
                f.canTeachSubjects.insert(subId);
                break;
            }
        }
    }

    for (const auto& item : unavailableList) {
        int facId = item.first;
        int slotId = item.second;
        if (facultyIds.find(facId) == facultyIds.end()) {
            errorMessage = "Referential integrity error: UNAVAILABLE references non-existent faculty ID " + std::to_string(facId);
            return false;
        }
        if (slotIds.find(slotId) == slotIds.end()) {
            errorMessage = "Referential integrity error: UNAVAILABLE references non-existent slot ID " + std::to_string(slotId);
            return false;
        }
        for (auto& f : newFaculty) {
            if (f.id == facId) {
                f.unavailableSlots.insert(slotId);
                break;
            }
        }
    }

    m_subjects = std::move(newSubjects);
    m_faculty = std::move(newFaculty);
    m_slots = std::move(newSlots);

    return true;
}

} // namespace core
