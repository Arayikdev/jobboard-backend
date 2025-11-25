#pragma once
#include "../third_party/httplib.h"
#include "../utils/jwt.hpp"
#include <string>

class AuthMiddleware {
public:
    // ---------------------------------------------------------
    // 1) COMPANY-ONLY ACCESS
    // ---------------------------------------------------------
    static bool verifyCompany(const httplib::Request &req,
                              httplib::Response &res,
                              std::string &outUserId) 
    {
        // Expect: Authorization: Bearer <token>
        auto auth = req.get_header_value("Authorization");
        if (auth.rfind("Bearer ", 0) != 0) {
            res.status = 401;
            res.set_content("Missing or invalid Authorization header", "text/plain");
            return false;
        }

        std::string token = auth.substr(7);

        try {
            auto decoded = JWT::verify(token);

            // Extract and validate role
            std::string role = decoded.get_payload_claim("role").as_string();
            if (role != "company") {
                res.status = 403;
                res.set_content("Access denied: only companies are allowed", "text/plain");
                return false;
            }

            // Extract userId
            outUserId = decoded.get_payload_claim("userId").as_string();
            return true;

        } catch (...) {
            res.status = 401;
            res.set_content("Invalid or expired token", "text/plain");
            return false;
        }
    }

    // ---------------------------------------------------------
    // 2) GENERAL USER ACCESS (optional for GET)
    // ---------------------------------------------------------
    static bool verifyUser(const httplib::Request &req,
                           httplib::Response &res,
                           std::string &outUserId)
    {
        auto auth = req.get_header_value("Authorization");

        // If no token → not authorized, but allowed for GET endpoints
        if (auth.rfind("Bearer ", 0) != 0) {
            return false; 
        }

        std::string token = auth.substr(7);

        try {
            auto decoded = JWT::verify(token);

            // Extract userId
            outUserId = decoded.get_payload_claim("userId").as_string();
            return true;

        } catch (...) {
            res.status = 401;
            res.set_content("Invalid or expired token", "text/plain");
            return false;
        }
    }
};
