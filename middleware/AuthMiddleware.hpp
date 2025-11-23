#pragma once
#include "../utils/jwt.hpp"
#include "../third_party/httplib.h"
#include <string>

class AuthMiddleware {
public:
    // -------------------------
    // COMPANY ONLY MIDDLEWARE
    // -------------------------
    static bool verifyCompany(const httplib::Request &req,
                              httplib::Response &res,
                              std::string &outUserId) 
    {
        const std::string secret = "SECRET_KEY";

        auto auth = req.get_header_value("Authorization");
        if (auth.rfind("Bearer ", 0) != 0) {
            res.status = 401;
            res.set_content("Missing or invalid Authorization header", "text/plain");
            return false;
        }

        std::string token = auth.substr(7);

        try {
            auto decoded = jwt::decode(token);

            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{secret})
                .with_issuer("cpp-backend")
                .verify(decoded);

            auto role = decoded.get_payload_claim("role").as_string();
            if (role != "company") {
                res.status = 403;
                res.set_content("Only companies can post jobs", "text/plain");
                return false;
            }

            outUserId = decoded.get_payload_claim("userId").as_string();
            return true;
        }
        catch (...) {
            res.status = 401;
            res.set_content("Invalid or expired token", "text/plain");
            return false;
        }
    }

    // -------------------------
    // GENERAL USER MIDDLEWARE
    // -------------------------
    static bool verifyUser(const httplib::Request &req,
                           httplib::Response &res,
                           std::string &outUserId)
    {
        const std::string secret = "SECRET_KEY";

        auto auth = req.get_header_value("Authorization");
        if (auth.rfind("Bearer ", 0) != 0) {
            return false; // No token → not logged in (but allowed for GET)
        }

        std::string token = auth.substr(7);

        try {
            auto decoded = jwt::decode(token);

            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{secret})
                .with_issuer("cpp-backend")
                .verify(decoded);

            outUserId = decoded.get_payload_claim("userId").as_string();
            return true;
        }
        catch (...) {
            res.status = 401;
            res.set_content("Invalid or expired token", "text/plain");
            return false;
        }
    }
};
