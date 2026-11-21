#include "core/block.hpp"
#include "core/sha256.hpp"

#include <algorithm>

namespace core {

std::string Block::sanitizeNote(const std::string& raw) {
    std::string clean = raw;
    for (char& c : clean) {
        if (c == '|' || c == '\n' || c == '\r') {
            c = ' ';
        }
    }
    return clean;
}

Block::Block(int index_,
             int64_t timestamp_,
             std::string note_,
             std::string data_,
             std::string prevHash_)
    : index(index_),
      timestamp(timestamp_),
      note(sanitizeNote(note_)),
      data(std::move(data_)),
      prevHash(std::move(prevHash_)),
      nonce(0) {
    hash = calculateHash();
}

std::string Block::calculateHash() const {
    // hash = sha256( index | timestamp | note | data | prevHash | nonce ) (literal '|')
    std::string payload = std::to_string(index) + "|" +
                          std::to_string(timestamp) + "|" +
                          sanitizeNote(note) + "|" +
                          data + "|" +
                          prevHash + "|" +
                          std::to_string(nonce);
    return sha256(payload);
}

bool Block::hasValidProofOfWork(int difficulty) const {
    if (static_cast<int>(hash.size()) < difficulty) {
        return false;
    }
    for (int i = 0; i < difficulty; ++i) {
        if (hash[i] != '0') return false;
    }
    return true;
}

void Block::mine(int difficulty) {
    nonce = 0;
    while (true) {
        hash = calculateHash();
        if (hasValidProofOfWork(difficulty)) {
            break;
        }
        nonce++;
    }
}

} // namespace core
