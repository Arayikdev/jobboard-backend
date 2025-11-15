#pragma once
#include "../third_party/httplib.h"
#include "../utils/jwt.hpp"

class AuthMiddleware {
public:
    static bool check(const httplib::Request &req, httplib::Response &res) {
        if (!req.has_header("Authorization")) {
            res.status = 401;
            res.set_content("{\"error\":\"Missing Authorization\"}", "application/json");
            return false;
        }

        std::string header = req.get_header_value("Authorization");
        if (header.rfind("Bearer ", 0) != 0) {
            res.status = 401;
            res.set_content("{\"error\":\"Invalid Authorization format\"}", "application/json");
            return false;
        }

        std::string token = header.substr(7);

        try {
            JWT::verify(token);
            return true; // токен валидный
        }
        catch (...) {
            res.status = 401;
            res.set_content("{\"error\":\"Invalid or expired token\"}", "application/json");
            return false;
        }
    }
};
