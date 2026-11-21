#ifndef TIMETABLE_CHAIN_SHA256_HPP
#define TIMETABLE_CHAIN_SHA256_HPP

#include <string>

namespace core {

/**
 * @brief Computes standard cryptographic SHA-256 hash.
 * 
 * Pure C++17 implementation following FIPS PUB 180-4 standard.
 * Produces a 64-character lowercase hexadecimal string.
 *
 * Self-check vector:
 * sha256("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
 */
std::string sha256(const std::string& input);

} // namespace core

#endif // TIMETABLE_CHAIN_SHA256_HPP
