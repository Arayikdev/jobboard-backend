#pragma once
#include "../models/User.hpp"
#include "../third_party/json.hpp"
#include <mongocxx/collection.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <string>
#include <array>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class UserAuthService
{
    mongocxx::collection collection;
    mongocxx::collection email_collection;

public:
    UserAuthService(mongocxx::collection coll, mongocxx::collection e_coll) : collection(coll), email_collection(e_coll) {}

    // ------------------------------------------------------------------
    // Register a new user
    // ------------------------------------------------------------------
    bool registerUser(const std::string &email, const std::string &password)
    {
        std::cout << "a" << std::endl;
        if (email.empty() || password.empty())
        {
            return false;
        }

        // Check if email already exists
        auto existing = email_collection.find_one(make_document(kvp("email", email)));
        if (existing)
        {
            return false; // email already taken
        }

        // Hash password
        std::string hashed = hashPassword(password);

        // Insert email into email_collection
        email_collection.insert_one(make_document(kvp("email", email)));
        std::cout << "email inserted" << std::endl;

        // Create user and insert into users collection
        
        User user(email, hashed);
        std::cout << "User created" << std::endl;
        collection.insert_one(user.toBson().view());
        std::cout << "User inserted" << std::endl;

        return true;
    }

    // ------------------------------------------------------------------
    // Verify login credentials
    // Returns true only if password matches
    // ------------------------------------------------------------------
    bool verifyPassword(const std::string &email, const std::string &passwordAttempt)
    {
        if (email.empty() || passwordAttempt.empty())
        {
            return false;
        }

        auto maybe_doc = collection.find_one(make_document(kvp("email", email)));
        if (!maybe_doc)
        {
            return false; // user not found
        }

        const auto &doc = *maybe_doc; // bsoncxx::document::view

        // This is the correct, modern way:
        std::string storedHash = std::string(doc["password"].get_string().value);

        return checkPassword(passwordAttempt, storedHash);
    }

private:
    // ------------------------------------------------------------------
    // Hash a new password (called only on registration)
    // Returns "hash:salt" (both hex strings)
    // ------------------------------------------------------------------
    std::string hashPassword(const std::string &password)
    {
        // 16-byte random salt
        std::array<unsigned char, 16> salt;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 255);

        for (auto &b : salt)
            b = static_cast<unsigned char>(dist(gen));

        return hashWithSalt(password, salt.data(), salt.size());
    }

    // ------------------------------------------------------------------
    // Core hashing function – used both for registration and verification
    // ------------------------------------------------------------------
    std::string hashWithSalt(const std::string &password,
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
        {
            out << std::setw(2) << static_cast<unsigned>(hash[i]);
        }
        out << ':';

        for (size_t i = 0; i < saltLen; ++i)
        {
            out << std::setw(2) << static_cast<unsigned>(salt[i]);
        }

        return out.str(); // format: 64-hex-chars : 32-hex-chars  (hash:salt)
    }

    // ------------------------------------------------------------------
    // Verify a password attempt against stored "hash:salt"
    // ------------------------------------------------------------------
    bool checkPassword(const std::string &passwordAttempt,
                       const std::string &stored) // stored = "hash:salt"
    {
        size_t colonPos = stored.find(':');
        if (colonPos == std::string::npos || colonPos + 33 > stored.size())
        {
            return false; // invalid format
        }

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

        // Constant-time comparison to prevent timing attacks
        return storedHash == candidateHash;
    }
};