#pragma once
#include "../models/User.hpp"
#include "../third_party/json.hpp"
#include <mongocxx/collection.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include "../utils/Password.hpp"

// #include <openssl/sha.h>
// #include <sstream>
// #include <iomanip>
// #include <random>
// #include <string>
// #include <array>

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
};