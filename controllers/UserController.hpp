#pragma once
#include <vector>
#include <bsoncxx/json.hpp>
#include "../services/UserService.hpp"
#include "../third_party/httplib.h"

using json = nlohmann::json;

class UserController
{
private:
    UserService &service;

public:
    UserController(UserService &srv) : service(srv) {}

    // GET /users
    void getUsers(const httplib::Request &req, httplib::Response &res)
    {
        try {
            auto users = service.getUsers();
            res.status = 200;
            res.set_content(json(users).dump(), "application/json");
        }
        catch (...) {
            res.status = 500;
            res.set_content("Server error", "text/plain");
        }
    }

    // GET /users/:id
    void getUserById(const httplib::Request &req,
                     httplib::Response &res,
                     const std::string &tokenUserId,
                     bool loggedIn)
    {
        std::string pathId = req.matches[1].str();

        // 1. Owner → return full profile
        if (loggedIn && tokenUserId == pathId) {
            auto fullUser = service.getOnePrivate(bsoncxx::oid(pathId));

            if (fullUser.is_null()) {
                res.status = 404;
                res.set_content("User not found", "text/plain");
                return;
            }

            res.status = 200;
            res.set_content(fullUser.dump(), "application/json");
            return;
        }

        // 2. Public profile
        auto publicUser = service.getOnePublic(bsoncxx::oid(pathId));

        if (publicUser.is_null()) {
            res.status = 404;
            res.set_content("User not found or private", "text/plain");
            return;
        }

        res.status = 200;
        res.set_content(publicUser.dump(), "application/json");
    }

    // PUT /users/:id
    void updateProfile(const httplib::Request &req,
                       httplib::Response &res,
                       const std::string &tokenUserId)
    {
        std::string pathId = req.matches[1].str();

        if (tokenUserId != pathId) {
            res.status = 403;
            res.set_content("Cannot update another user's profile", "text/plain");
            return;
        }

        json body;
        try {
            body = json::parse(req.body);
        }
        catch (...) {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
            return;
        }

        bool ok = service.updateProfile(pathId, body);

        if (!ok) {
            res.status = 404;
            res.set_content("User not found", "text/plain");
            return;
        }

        res.status = 200;
        res.set_content("Updated successfully", "text/plain");
    }
};
