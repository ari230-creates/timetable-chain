#include "httplib.h"
#include "json.hpp"

#include "core/dataset.hpp"
#include "core/generator.hpp"
#include "core/constraint_checker.hpp"
#include "core/scorer.hpp"
#include "core/blockchain.hpp"
#include "core/block.hpp"
#include "core/timetable.hpp"
#include "core/sha256.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>

using json = nlohmann::json;

namespace fs = std::filesystem;

namespace {

int64_t getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string readFileToString(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return "";
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

json datasetToJson(const core::Dataset& ds) {
    json j;
    json subjects = json::array();
    for (const auto& s : ds.getSubjects()) {
        subjects.push_back({
            {"id", s.id},
            {"name", s.name},
            {"periodsPerWeek", s.periodsPerWeek}
        });
    }
    j["subjects"] = subjects;

    json faculty = json::array();
    for (const auto& f : ds.getFaculty()) {
        std::vector<int> canTeach(f.canTeachSubjects.begin(), f.canTeachSubjects.end());
        std::vector<int> unavail(f.unavailableSlots.begin(), f.unavailableSlots.end());
        faculty.push_back({
            {"id", f.id},
            {"name", f.name},
            {"canTeach", canTeach},
            {"unavailable", unavail}
        });
    }
    j["faculty"] = faculty;

    json slots = json::array();
    for (const auto& slot : ds.getSlots()) {
        slots.push_back({
            {"id", slot.id},
            {"day", slot.day},
            {"period", slot.period},
            {"name", slot.toString()}
        });
    }
    j["slots"] = slots;

    j["totalPeriodsNeeded"] = ds.totalPeriodsNeeded();
    j["totalSlotsAvailable"] = ds.getSlots().size();
    j["isFeasible"] = (ds.totalPeriodsNeeded() <= static_cast<int>(ds.getSlots().size()));
    return j;
}

json entriesToJson(const core::Timetable& tt) {
    json arr = json::array();
    for (const auto& e : tt.getEntries()) {
        arr.push_back({
            {"subjectId", e.subjectId},
            {"facultyId", e.facultyId},
            {"slotId", e.slotId}
        });
    }
    return arr;
}

core::Timetable jsonToTimetable(const json& entriesJson) {
    core::Timetable tt;
    if (entriesJson.is_array()) {
        for (const auto& item : entriesJson) {
            int subId = item.value("subjectId", 0);
            int facId = item.value("facultyId", 0);
            int slotId = item.value("slotId", 0);
            tt.addEntry({subId, facId, slotId});
        }
    }
    return tt;
}

json blockToJson(const core::Block& b) {
    return {
        {"index", b.index},
        {"timestamp", b.timestamp},
        {"note", b.note},
        {"data", b.data},
        {"prevHash", b.prevHash},
        {"nonce", b.nonce},
        {"hash", b.hash}
    };
}

core::Block jsonToBlock(const json& j) {
    core::Block b;
    b.index = j.value("index", 0);
    b.timestamp = j.value("timestamp", (int64_t)0);
    b.note = j.value("note", "");
    b.data = j.value("data", "");
    b.prevHash = j.value("prevHash", "");
    b.nonce = j.value("nonce", (uint64_t)0);
    b.hash = j.value("hash", "");
    return b;
}

void setCorsHeaders(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    int port = 8080;
    if (const char* envPort = std::getenv("PORT")) {
        port = std::atoi(envPort);
    }
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    httplib::Server svr;

    // CORS preflight handler
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        res.status = 204;
    });

    // Logger
    svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        std::cout << "[HTTP] " << req.method << " " << req.path << " -> " << res.status << std::endl;
    });

    // 1. Health check
    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        json out = {
            {"status", "ok"},
            {"app", "TIMETABLE CHAIN"},
            {"version", "1.0.0"},
            {"engine", "C++17 libcore (Native)"}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 2. Sample datasets
    svr.Get("/api/sample", [](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        std::string sample = readFileToString("data/sample.txt");
        if (sample.empty()) sample = readFileToString("../data/sample.txt");
        json out = {
            {"name", "sample.txt"},
            {"content", sample}
        };
        res.set_content(out.dump(), "application/json");
    });

    svr.Get("/api/sample-full-week", [](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        std::string sample = readFileToString("data/sample_full_week.txt");
        if (sample.empty()) sample = readFileToString("../data/sample_full_week.txt");
        json out = {
            {"name", "sample_full_week.txt"},
            {"content", sample}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 3. Parse dataset
    svr.Post("/api/dataset/parse", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("content")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'content'"}}.dump(), "application/json");
            return;
        }

        std::string content = in["content"].get<std::string>();
        core::Dataset ds;
        std::string err;
        bool ok = ds.loadFromString(content, err);

        json out;
        if (!ok) {
            out["valid"] = false;
            out["error"] = err;
        } else {
            out["valid"] = true;
            out["error"] = "";
            out["dataset"] = datasetToJson(ds);
        }
        res.set_content(out.dump(), "application/json");
    });

    // 4. Validate dataset with detailed diagnostics
    svr.Post("/api/dataset/validate", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("content")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'content'"}}.dump(), "application/json");
            return;
        }

        std::string content = in["content"].get<std::string>();
        core::Dataset ds;
        std::string err;
        bool ok = ds.loadFromString(content, err);

        std::vector<std::string> errors;
        std::vector<std::string> warnings;

        if (!ok) {
            errors.push_back(err);
        } else {
            // Check domain consistency
            if (ds.getSubjects().empty()) {
                warnings.push_back("Dataset defines no subjects.");
            }
            if (ds.getFaculty().empty()) {
                warnings.push_back("Dataset defines no faculty members.");
            }
            if (ds.getSlots().empty()) {
                warnings.push_back("Dataset defines no weekly time slots.");
            }

            int needed = ds.totalPeriodsNeeded();
            int avail = static_cast<int>(ds.getSlots().size());
            if (needed > avail) {
                errors.push_back("Deficit: Required periods (" + std::to_string(needed) +
                                  ") exceed total available time slots (" + std::to_string(avail) + ").");
            }

            // Check if every subject has at least one qualified faculty member
            for (const auto& sub : ds.getSubjects()) {
                if (sub.periodsPerWeek <= 0) {
                    warnings.push_back("Subject '" + sub.name + "' has 0 periods per week.");
                }
                int qualifiedCount = 0;
                for (const auto& f : ds.getFaculty()) {
                    if (f.canTeach(sub.id)) qualifiedCount++;
                }
                if (qualifiedCount == 0 && sub.periodsPerWeek > 0) {
                    errors.push_back("Unteachable Subject: No faculty is qualified to teach '" +
                                     sub.name + "' (ID " + std::to_string(sub.id) + ").");
                }
            }

            // Check faculty availability vs requirements
            for (const auto& f : ds.getFaculty()) {
                if (f.canTeachSubjects.empty()) {
                    warnings.push_back("Faculty '" + f.name + "' (ID " + std::to_string(f.id) + ") is not assigned to any subject.");
                }
                if (f.unavailableSlots.size() == ds.getSlots().size() && !ds.getSlots().empty()) {
                    warnings.push_back("Faculty '" + f.name + "' is unavailable in all time slots.");
                }
            }
        }

        json out = {
            {"valid", errors.empty()},
            {"errors", errors},
            {"warnings", warnings},
            {"stats", {
                {"subjects", ds.getSubjects().size()},
                {"faculty", ds.getFaculty().size()},
                {"slots", ds.getSlots().size()},
                {"periodsNeeded", ds.totalPeriodsNeeded()},
                {"slotsAvailable", ds.getSlots().size()}
            }}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 5. Generate timetable
    svr.Post("/api/generate", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("content")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'content'"}}.dump(), "application/json");
            return;
        }

        std::string content = in["content"].get<std::string>();
        std::string modeStr = in.value("mode", "MRV");
        bool mineAfter = in.value("mineAfter", false);
        std::string note = in.value("note", "Generated Schedule");
        std::string prevHash = in.value("prevHash", "0");
        int blockIndex = in.value("index", 1);

        core::GeneratorMode mode = core::GeneratorMode::MRV;
        if (modeStr == "Plain") mode = core::GeneratorMode::Plain;
        else if (modeStr == "Best") mode = core::GeneratorMode::Best;

        core::Dataset ds;
        std::string err;
        if (!ds.loadFromString(content, err)) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", "Failed to parse dataset: " + err}}.dump(), "application/json");
            return;
        }

        core::Generator gen(ds);
        core::Timetable tt;

        auto t0 = std::chrono::high_resolution_clock::now();
        bool ok = gen.run(mode, tt);
        auto t1 = std::chrono::high_resolution_clock::now();
        double durationMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

        json out;
        out["success"] = ok;
        if (!ok) {
            out["error"] = "No feasible schedule found satisfying all constraints.";
            out["stats"] = {
                {"placements", gen.getStats().placements},
                {"backtracks", gen.getStats().backtracks},
                {"score", gen.getStats().score},
                {"solutionsFound", gen.getStats().solutionsFound},
                {"durationMs", durationMs}
            };
        } else {
            out["entries"] = entriesToJson(tt);
            out["serialized"] = tt.serialize();
            out["stats"] = {
                {"placements", gen.getStats().placements},
                {"backtracks", gen.getStats().backtracks},
                {"score", gen.getStats().score},
                {"solutionsFound", gen.getStats().solutionsFound},
                {"durationMs", durationMs}
            };

            if (mineAfter) {
                int64_t now = getCurrentTimestamp();
                core::Block b(blockIndex, now, note, tt.serialize(), prevHash);
                b.mine(core::Block::DIFFICULTY);
                out["minedBlock"] = blockToJson(b);
            }
        }

        res.set_content(out.dump(), "application/json");
    });

    // 6. Compare strategies
    svr.Post("/api/compare", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("content")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'content'"}}.dump(), "application/json");
            return;
        }

        std::string content = in["content"].get<std::string>();
        core::Dataset ds;
        std::string err;
        if (!ds.loadFromString(content, err)) {
            res.status = 400;
            res.set_content(json{{"error", "Failed to parse dataset: " + err}}.dump(), "application/json");
            return;
        }

        json results = json::array();

        // Plain
        {
            core::Generator gen(ds);
            core::Timetable tt;
            auto t0 = std::chrono::high_resolution_clock::now();
            bool ok = gen.run(core::GeneratorMode::Plain, tt);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            results.push_back({
                {"name", "Plain"},
                {"description", "Order tasks linearly by file sequence"},
                {"success", ok},
                {"placements", gen.getStats().placements},
                {"backtracks", gen.getStats().backtracks},
                {"score", gen.getStats().score},
                {"solutionsFound", gen.getStats().solutionsFound},
                {"durationMs", ms}
            });
        }

        // MRV
        {
            core::Generator gen(ds);
            core::Timetable tt;
            auto t0 = std::chrono::high_resolution_clock::now();
            bool ok = gen.run(core::GeneratorMode::MRV, tt);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            results.push_back({
                {"name", "MRV"},
                {"description", "Minimum Remaining Values: prioritize most constrained courses first"},
                {"success", ok},
                {"placements", gen.getStats().placements},
                {"backtracks", gen.getStats().backtracks},
                {"score", gen.getStats().score},
                {"solutionsFound", gen.getStats().solutionsFound},
                {"durationMs", ms}
            });
        }

        // Best
        {
            core::Generator gen(ds);
            core::Timetable tt;
            auto t0 = std::chrono::high_resolution_clock::now();
            bool ok = gen.run(core::GeneratorMode::Best, tt);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            results.push_back({
                {"name", "Best"},
                {"description", "MRV variable ordering exploring up to 50 solutions for minimum penalty"},
                {"success", ok},
                {"placements", gen.getStats().placements},
                {"backtracks", gen.getStats().backtracks},
                {"score", gen.getStats().score},
                {"solutionsFound", gen.getStats().solutionsFound},
                {"durationMs", ms}
            });
        }

        json out = {
            {"results", results}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 7. Manual Builder can-place constraint check
    svr.Post("/api/manual/can-place", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("content")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'content'"}}.dump(), "application/json");
            return;
        }

        std::string content = in["content"].get<std::string>();
        core::Dataset ds;
        std::string err;
        if (!ds.loadFromString(content, err)) {
            res.status = 400;
            res.set_content(json{{"error", "Failed to parse dataset: " + err}}.dump(), "application/json");
            return;
        }

        core::Timetable tt = jsonToTimetable(in.value("entries", json::array()));
        int subId = in.value("subjectId", 0);
        int facId = in.value("facultyId", 0);
        int slotId = in.value("slotId", 0);

        const auto* sub = ds.findSubject(subId);
        const auto* fac = ds.findFaculty(facId);
        const auto* slot = ds.findSlot(slotId);

        if (!sub || !fac || !slot) {
            json out = {
                {"valid", false},
                {"reason", "Unknown entity reference (subject, faculty, or slot)"}
            };
            res.set_content(out.dump(), "application/json");
            return;
        }

        // If target slot is already assigned to something else, remove it first for testing replacement
        tt.removeEntry(slotId);

        std::string reason;
        bool ok = core::ConstraintChecker::canPlace(tt, *sub, *fac, *slot, reason);

        json out = {
            {"valid", ok},
            {"reason", reason}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 8. Manual Builder validate whole schedule
    svr.Post("/api/manual/validate", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("content")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'content'"}}.dump(), "application/json");
            return;
        }

        std::string content = in["content"].get<std::string>();
        core::Dataset ds;
        std::string err;
        if (!ds.loadFromString(content, err)) {
            res.status = 400;
            res.set_content(json{{"error", "Failed to parse dataset: " + err}}.dump(), "application/json");
            return;
        }

        core::Timetable tt = jsonToTimetable(in.value("entries", json::array()));
        std::vector<std::string> problems;
        bool valid = core::ConstraintChecker::isValid(tt, ds, problems);
        int score = core::Scorer::calculate(tt, ds);

        json out = {
            {"valid", valid},
            {"problems", problems},
            {"penaltyScore", score},
            {"serialized", tt.serialize()}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 9. Mine block
    svr.Post("/api/blockchain/mine", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded()) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON"}}.dump(), "application/json");
            return;
        }

        int index = in.value("index", 1);
        int64_t timestamp = in.value("timestamp", getCurrentTimestamp());
        std::string note = in.value("note", "Timetable Block");
        std::string data = in.value("data", "");
        std::string prevHash = in.value("prevHash", "0");

        core::Block b(index, timestamp, note, data, prevHash);
        b.mine(core::Block::DIFFICULTY);

        json out = blockToJson(b);
        res.set_content(out.dump(), "application/json");
    });

    // 10. Verify blockchain (3-layer verification)
    svr.Post("/api/blockchain/verify", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("content")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'content'"}}.dump(), "application/json");
            return;
        }

        std::string content = in["content"].get<std::string>();
        core::Dataset ds;
        std::string err;
        if (!ds.loadFromString(content, err)) {
            res.status = 400;
            res.set_content(json{{"error", "Failed to parse dataset: " + err}}.dump(), "application/json");
            return;
        }

        core::Blockchain chain;
        auto& blocks = chain.getBlocks();
        blocks.clear();

        if (in.contains("blocks") && in["blocks"].is_array()) {
            for (const auto& bj : in["blocks"]) {
                blocks.push_back(jsonToBlock(bj));
            }
        }

        if (blocks.empty()) {
            chain.reset(); // create default genesis
        }

        core::VerificationReport report = chain.verify(ds);

        json statusMap = json::object();
        for (const auto& [idx, st] : report.blockStatuses) {
            statusMap[std::to_string(idx)] = {
                {"index", st.index},
                {"valid", st.valid},
                {"problems", st.problems}
            };
        }

        json out = {
            {"allValid", report.allValid},
            {"messages", report.messages},
            {"blockStatuses", statusMap}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 11. Tamper block demonstration
    svr.Post("/api/blockchain/tamper", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("blocks")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'blocks'"}}.dump(), "application/json");
            return;
        }

        core::Blockchain chain;
        auto& blocks = chain.getBlocks();
        blocks.clear();
        for (const auto& bj : in["blocks"]) {
            blocks.push_back(jsonToBlock(bj));
        }

        int targetIndex = in.value("targetIndex", 1);
        bool remine = in.value("remine", false);
        std::string customData = in.value("customData", "");

        if (targetIndex < 0 || targetIndex >= static_cast<int>(blocks.size())) {
            res.status = 400;
            res.set_content(json{{"error", "Target block index out of range"}}.dump(), "application/json");
            return;
        }

        if (customData.empty()) {
            if (blocks[targetIndex].data == "GENESIS") {
                customData = "TAMPERED_GENESIS";
            } else {
                core::Timetable tt = core::Timetable::deserialize(blocks[targetIndex].data);
                auto entries = tt.getEntries();
                if (!entries.empty()) {
                    // Alter faculty of first entry
                    entries[0].facultyId = (entries[0].facultyId == 1) ? 2 : 1;
                    core::Timetable tamperedTt;
                    for (const auto& e : entries) tamperedTt.addEntry(e);
                    customData = tamperedTt.serialize();
                } else {
                    customData = "99,99,99;";
                }
            }
        }

        chain.tamperBlock(targetIndex, customData, remine);

        json outBlocks = json::array();
        for (const auto& b : chain.getBlocks()) {
            outBlocks.push_back(blockToJson(b));
        }

        json out = {
            {"success", true},
            {"tamperedIndex", targetIndex},
            {"remined", remine},
            {"blocks", outBlocks}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 12. Save blockchain file format
    svr.Post("/api/blockchain/save", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("blocks")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'blocks'"}}.dump(), "application/json");
            return;
        }

        core::Blockchain chain;
        auto& blocks = chain.getBlocks();
        blocks.clear();
        for (const auto& bj : in["blocks"]) {
            blocks.push_back(jsonToBlock(bj));
        }

        std::string fileContent = chain.saveToString();
        json out = {
            {"content", fileContent}
        };
        res.set_content(out.dump(), "application/json");
    });

    // 13. Load blockchain file format
    svr.Post("/api/blockchain/load", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("content")) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON or missing 'content'"}}.dump(), "application/json");
            return;
        }

        std::string content = in["content"].get<std::string>();
        core::Blockchain chain;
        std::string err;
        bool ok = chain.loadFromString(content, err);

        if (!ok) {
            res.status = 400;
            json out = {
                {"success", false},
                {"error", err}
            };
            res.set_content(out.dump(), "application/json");
            return;
        }

        json outBlocks = json::array();
        for (const auto& b : chain.getBlocks()) {
            outBlocks.push_back(blockToJson(b));
        }

        json out = {
            {"success", true},
            {"blocks", outBlocks}
        };
        res.set_content(out.dump(), "application/json");
    });

    // Static file serving: check if web/dist exists
    std::string webDir = "web/dist";
    if (!fs::exists(webDir) && fs::exists("../web/dist")) {
        webDir = "../web/dist";
    }
    if (fs::exists(webDir)) {
        svr.set_mount_point("/", webDir);
        // Fallback for SPA routing
        svr.set_error_handler([webDir](const httplib::Request& req, httplib::Response& res) {
            if (res.status == 404 && req.method == "GET" && req.path.rfind("/api/", 0) != 0) {
                std::string indexPath = webDir + "/index.html";
                std::string indexHtml = readFileToString(indexPath);
                if (!indexHtml.empty()) {
                    res.status = 200;
                    res.set_content(indexHtml, "text/html");
                }
            }
        });
        std::cout << "[SERVER] Serving static frontend from " << webDir << std::endl;
    }

    std::cout << "====================================================\n";
    std::cout << "  TIMETABLE CHAIN — HTTP Backend Server\n";
    std::cout << "  Listening on http://localhost:" << port << "\n";
    std::cout << "  C++ Core Engine: Active\n";
    std::cout << "====================================================\n";

    svr.listen("0.0.0.0", port);

    return 0;
}
