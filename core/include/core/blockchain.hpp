#ifndef TIMETABLE_CHAIN_BLOCKCHAIN_HPP
#define TIMETABLE_CHAIN_BLOCKCHAIN_HPP

#include "core/block.hpp"
#include "core/dataset.hpp"
#include <vector>
#include <string>
#include <map>

namespace core {

/**
 * @brief Verification status for an individual block.
 */
struct BlockVerificationStatus {
    int index = 0;
    bool valid = true;
    std::vector<std::string> problems;
};

/**
 * @brief Comprehensive 3-layer verification report for the entire blockchain.
 */
struct VerificationReport {
    bool allValid = true;
    std::vector<std::string> messages; // All problems formatted as "Block N: ..."
    std::map<int, BlockVerificationStatus> blockStatuses;

    bool isBlockValid(int blockIndex) const {
        auto it = blockStatuses.find(blockIndex);
        if (it != blockStatuses.end()) {
            return it->second.valid;
        }
        return false;
    }
};

/**
 * @brief Manages the append-only ledger of verified timetable blocks.
 */
class Blockchain {
public:
    Blockchain();

    /// Returns the list of blocks.
    const std::vector<Block>& getBlocks() const { return m_blocks; }
    std::vector<Block>& getBlocks() { return m_blocks; }

    /// Returns the number of blocks in the chain.
    size_t size() const { return m_blocks.size(); }

    /// Clears and initializes fresh chain with genesis block.
    void reset();

    /**
     * @brief Appends a new block to the chain after mining proof-of-work.
     * 
     * Rule: Never append a block to a chain that fails verification.
     * 
     * @param note Human-readable description.
     * @param data Serialized timetable string.
     * @param dataset Curricular dataset to verify chain before appending.
     * @param errorOut Populated with error message if verification fails.
     * @return true if appended successfully, false if rejected.
     */
    bool addBlock(const std::string& note,
                  const std::string& data,
                  const Dataset& dataset,
                  std::string* errorOut = nullptr);

    /**
     * @brief Performs comprehensive 3-layer verification.
     * 
     * Layer 1 (Integrity): index match, hash validity, proof-of-work difficulty.
     * Layer 2 (Links): prevHash matches previous block's hash.
     * Layer 3 (Meaning): deserialized timetable satisfies all curricular rules.
     */
    VerificationReport verify(const Dataset& dataset) const;

    /**
     * @brief Saves chain to disk file in TIMETABLE_CHAIN_V1 format.
     */
    bool save(const std::string& path, std::string& errorMessage) const;

    /**
     * @brief Serializes chain to string in TIMETABLE_CHAIN_V1 format.
     */
    std::string saveToString() const;

    /**
     * @brief Loads chain from disk file. Does NOT auto-verify.
     */
    bool load(const std::string& path, std::string& errorMessage);

    /**
     * @brief Loads chain from string in TIMETABLE_CHAIN_V1 format.
     */
    bool loadFromString(const std::string& content, std::string& errorMessage);

    /**
     * @brief Tamper simulator for educational demonstration.
     * 
     * @param blockIndex Target block to corrupt.
     * @param newData Altered timetable serialization.
     * @param remine If true (clever attacker), recalculates nonce and hash.
     *               If false (careless attacker), leaves hash intact causing mismatch.
     */
    bool tamperBlock(int blockIndex, const std::string& newData, bool remine);

private:
    std::vector<Block> m_blocks;
    void createGenesisBlock();
};

} // namespace core

#endif // TIMETABLE_CHAIN_BLOCKCHAIN_HPP
