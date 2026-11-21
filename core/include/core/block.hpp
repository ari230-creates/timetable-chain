#ifndef TIMETABLE_CHAIN_BLOCK_HPP
#define TIMETABLE_CHAIN_BLOCK_HPP

#include <string>
#include <cstdint>

namespace core {

/**
 * @brief Represents a single immutable block within the timetable blockchain.
 */
class Block {
public:
    static constexpr int DIFFICULTY = 3;

    int index = 0;
    int64_t timestamp = 0;
    std::string note;
    std::string data;
    std::string prevHash;
    uint64_t nonce = 0;
    std::string hash;

    Block() = default;
    Block(int index_,
          int64_t timestamp_,
          std::string note_,
          std::string data_,
          std::string prevHash_);

    /// Sanitizes note by replacing '|' and newlines with spaces.
    static std::string sanitizeNote(const std::string& raw);

    /// Calculates SHA-256 hash: sha256( index | timestamp | note | data | prevHash | nonce )
    std::string calculateHash() const;

    /// Checks if current hash satisfies the proof-of-work difficulty.
    bool hasValidProofOfWork(int difficulty = DIFFICULTY) const;

    /// Solves proof-of-work by finding nonce such that hash starts with required zeroes.
    void mine(int difficulty = DIFFICULTY);
};

} // namespace core

#endif // TIMETABLE_CHAIN_BLOCK_HPP
