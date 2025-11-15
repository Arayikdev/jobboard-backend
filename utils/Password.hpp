#pragma once
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <string>
#include <array>

// ------------------------------------------------------------------
// Core hashing function – used both for registration and verification
// ------------------------------------------------------------------
inline std::string hashWithSalt(const std::string &password,
                                const unsigned char *salt,
                                size_t saltLen)
{
    std::string data = password;
    data.append(reinterpret_cast<const char *>(salt), saltLen);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(data.c_str()),
           data.length(),
           hash);

    std::ostringstream out;
    out << std::hex << std::setfill('0');

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        out << std::setw(2) << static_cast<unsigned>(hash[i]);

    out << ':';

    for (size_t i = 0; i < saltLen; ++i)
        out << std::setw(2) << static_cast<unsigned>(salt[i]);

    return out.str(); // format: hash:salt
}

// ------------------------------------------------------------------
// Hash a password with random salt
// ------------------------------------------------------------------
inline std::string hashPassword(const std::string &password)
{
    std::array<unsigned char, 16> salt{};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    for (auto &b : salt)
        b = static_cast<unsigned char>(dist(gen));

    return hashWithSalt(password, salt.data(), salt.size());
}

// ------------------------------------------------------------------
// Verify a password attempt against stored "hash:salt"
// ------------------------------------------------------------------
inline bool checkPassword(const std::string &passwordAttempt,
                          const std::string &stored) // stored = "hash:salt"
{
    size_t colonPos = stored.find(':');
    if (colonPos == std::string::npos || colonPos + 33 > stored.size())
        return false;

    std::string storedHash = stored.substr(0, colonPos);
    std::string saltHex = stored.substr(colonPos + 1);

    // Convert hex salt back to bytes
    std::array<unsigned char, 16> salt{};
    for (size_t i = 0; i < 32; i += 2)
    {
        std::string byteStr = saltHex.substr(i, 2);
        salt[i / 2] = static_cast<unsigned char>(std::stoul(byteStr, nullptr, 16));
    }

    std::string candidate = hashWithSalt(passwordAttempt, salt.data(), salt.size());
    std::string candidateHash = candidate.substr(0, colonPos);

    return storedHash == candidateHash; // constant-time comparison
}
