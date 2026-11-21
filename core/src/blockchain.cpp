#include "core/blockchain.hpp"
#include "core/constraint_checker.hpp"
#include "core/timetable.hpp"

#include <chrono>
#include <fstream>
#include <sstream>

namespace core {

namespace {

int64_t getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

} // anonymous namespace

Blockchain::Blockchain() {
    createGenesisBlock();
}

void Blockchain::createGenesisBlock() {
    m_blocks.clear();
    Block genesis(0, 1704067200, "Genesis Block", "GENESIS", "0");
    genesis.mine(Block::DIFFICULTY);
    m_blocks.push_back(genesis);
}

void Blockchain::reset() {
    createGenesisBlock();
}

bool Blockchain::addBlock(const std::string& note,
                          const std::string& data,
                          const Dataset& dataset,
                          std::string* errorOut) {
    // Rule: Never append a block to a chain that fails verification
    VerificationReport preCheck = verify(dataset);
    if (!preCheck.allValid) {
        if (errorOut) {
            *errorOut = "Cannot append block: current blockchain verification failed.";
        }
        return false;
    }

    int nextIndex = static_cast<int>(m_blocks.size());
    std::string prevHash = m_blocks.empty() ? "0" : m_blocks.back().hash;
    int64_t now = getCurrentTimestamp();

    Block newBlock(nextIndex, now, note, data, prevHash);
    newBlock.mine(Block::DIFFICULTY);
    m_blocks.push_back(newBlock);
    return true;
}

VerificationReport Blockchain::verify(const Dataset& dataset) const {
    VerificationReport report;
    report.allValid = true;

    for (size_t i = 0; i < m_blocks.size(); ++i) {
        const auto& b = m_blocks[i];
        BlockVerificationStatus status;
        status.index = b.index;
        status.valid = true;

        // --- Layer 1: Integrity ---
        // 1. Wrong index
        if (b.index != static_cast<int>(i)) {
            std::string msg = "Block " + std::to_string(b.index) +
                              ": Invalid block index (expected " + std::to_string(i) + ")";
            status.problems.push_back(msg);
            status.valid = false;
        }

        // 2. Recomputed hash != stored hash
        std::string computedHash = b.calculateHash();
        if (computedHash != b.hash) {
            std::string msg = "Block " + std::to_string(b.index) +
                              ": Hash mismatch (stored " + b.hash +
                              " != recomputed " + computedHash + ")";
            status.problems.push_back(msg);
            status.valid = false;
        }

        // 3. Proof-of-work invalid
        if (!b.hasValidProofOfWork(Block::DIFFICULTY)) {
            std::string msg = "Block " + std::to_string(b.index) +
                              ": Invalid proof of work (hash lacks required prefix '000')";
            status.problems.push_back(msg);
            status.valid = false;
        }

        // --- Layer 2: Links ---
        if (i == 0) {
            if (b.prevHash != "0") {
                std::string msg = "Block 0: Genesis block prevHash must be '0' (got " + b.prevHash + ")";
                status.problems.push_back(msg);
                status.valid = false;
            }
        } else {
            if (b.prevHash != m_blocks[i - 1].hash) {
                std::string msg = "Block " + std::to_string(b.index) +
                                  ": Broken link (prevHash " + b.prevHash +
                                  " != previous block hash " + m_blocks[i - 1].hash + ")";
                status.problems.push_back(msg);
                status.valid = false;
            }
        }

        // --- Layer 3: Meaning ---
        if (i > 0) {
            Timetable tt = Timetable::deserialize(b.data);
            std::vector<std::string> ttProblems;
            if (!ConstraintChecker::isValid(tt, dataset, ttProblems)) {
                status.valid = false;
                for (const auto& prob : ttProblems) {
                    std::string msg = "Block " + std::to_string(b.index) + ": " + prob;
                    status.problems.push_back(msg);
                }
            }
        }

        if (!status.valid) {
            report.allValid = false;
            for (const auto& p : status.problems) {
                report.messages.push_back(p);
            }
        }

        report.blockStatuses[b.index] = status;
    }

    return report;
}

std::string Blockchain::saveToString() const {
    std::ostringstream out;
    out << "TIMETABLE_CHAIN_V1\n";
    for (const auto& b : m_blocks) {
        out << "BLOCK|"
            << b.index << "|"
            << b.timestamp << "|"
            << b.nonce << "|"
            << b.prevHash << "|"
            << b.hash << "|"
            << Block::sanitizeNote(b.note) << "|"
            << b.data << "\n";
    }
    return out.str();
}

bool Blockchain::save(const std::string& path, std::string& errorMessage) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        errorMessage = "Unable to open file for writing: " + path;
        return false;
    }
    out << saveToString();
    return true;
}

bool Blockchain::loadFromString(const std::string& content, std::string& errorMessage) {
    std::istringstream in(content);
    std::string header;
    if (!std::getline(in, header) || header.rfind("TIMETABLE_CHAIN_V1", 0) != 0) {
        errorMessage = "Invalid chain format: missing TIMETABLE_CHAIN_V1 header";
        return false;
    }

    std::vector<Block> loadedBlocks;
    std::string line;
    int lineNum = 1;

    while (std::getline(in, line)) {
        lineNum++;
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> parts;
        while (std::getline(ss, token, '|')) {
            parts.push_back(token);
        }

        if (parts.size() < 8 || parts[0] != "BLOCK") {
            errorMessage = "Line " + std::to_string(lineNum) + ": Malformed BLOCK record";
            return false;
        }

        try {
            Block b;
            b.index = std::stoi(parts[1]);
            b.timestamp = std::stoll(parts[2]);
            b.nonce = std::stoull(parts[3]);
            b.prevHash = parts[4];
            b.hash = parts[5];
            b.note = parts[6];
            b.data = parts[7];
            loadedBlocks.push_back(b);
        } catch (const std::exception& ex) {
            errorMessage = "Line " + std::to_string(lineNum) + ": Parse error (" + ex.what() + ")";
            return false;
        }
    }

    if (loadedBlocks.empty()) {
        errorMessage = "Chain file contains no blocks";
        return false;
    }

    // Explicit constraint: Loading must NOT auto-verify; verification is an explicit step.
    m_blocks = std::move(loadedBlocks);
    return true;
}

bool Blockchain::load(const std::string& path, std::string& errorMessage) {
    std::ifstream in(path);
    if (!in.is_open()) {
        errorMessage = "Unable to open file for reading: " + path;
        return false;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    return loadFromString(buffer.str(), errorMessage);
}

bool Blockchain::tamperBlock(int blockIndex, const std::string& newData, bool remine) {
    if (blockIndex < 0 || blockIndex >= static_cast<int>(m_blocks.size())) {
        return false;
    }

    m_blocks[blockIndex].data = newData;

    if (remine) {
        // Clever attacker: recalculates nonce and hash to satisfy proof-of-work
        m_blocks[blockIndex].mine(Block::DIFFICULTY);
    }
    // If remine is false (careless attacker): data changed but stored hash is old, causing mismatch

    return true;
}

} // namespace core
