#pragma once
#include "../utils/jwt.hpp"
#include "../third_party/httplib.h"

class AdminMiddleware {
public:

    static bool check(const httplib::Request &req, httplib::Response &res) 
    {
        if (!req.has_header("Authorization")) {
            res.status = 401;
            res.set_content("Missing Authorization header", "text/plain");
            return false;
        }

        std::string auth = req.get_header_value("Authorization");

        if (auth.rfind("Bearer ", 0) != 0) {
            res.status = 401;
            res.set_content("Invalid token format", "text/plain");
            return false;
        }

        std::string token = auth.substr(7);

        try {
            auto decoded = JWT::verify(token);

            std::string role = decoded.get_payload_claim("role").as_string();
            if (role != "admin") {
                res.status = 403;
                res.set_content("Forbidden: Admins only", "text/plain");
                return false;
            }

            return true;
        }
        catch (...) {
            res.status = 401;
            res.set_content("Invalid or expired token", "text/plain");
            return false;
        }
    }
};
