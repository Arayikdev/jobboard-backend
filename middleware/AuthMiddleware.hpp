#pragma once
#include "../utils/jwt.hpp"
#include "../third_party/httplib.h"
#include <string>

class AuthMiddleware {
public:
    static bool verifyCompany(const httplib::Request &req,
                              httplib::Response &res,
                              std::string &outUserId) 
    {
        const std::string secret = "SECRET_KEY";

        // 1) Check header
        auto auth = req.get_header_value("Authorization");
        if (auth.rfind("Bearer ", 0) != 0) {
            res.status = 401;
            res.set_content("Missing or invalid Authorization header", "text/plain");
            return false;
        }

        std::string token = auth.substr(7);

        try {
            std::cout << "before decode" << std::endl;
            // 2) Decode
            auto decoded = jwt::decode(token);
            std::cout << "after decode" << std::endl;

            // 3) Verify
            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{secret})
                .with_issuer("cpp-backend")
                .verify(decoded);
            std::cout << "after verofy" << std::endl;

            // 4) Extract role
            auto role = decoded.get_payload_claim("role").as_string();
            if (role != "company") {
                res.status = 403;
                res.set_content("Only companies can post jobs", "text/plain");
                return false;
            }
            std::cout << "after get role" << std::endl;


            // 5) Extract userId
            outUserId = decoded.get_payload_claim("userId").as_string();
            std::cout << "after get userid" << std::endl;
            
            return true;
        }
        catch (...) {
            res.status = 401;
            res.set_content("Invalid or expired token", "text/plain");
            return false;
        }
    }
};
