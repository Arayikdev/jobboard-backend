#include <vector>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include "../services/UserService.hpp"
#include "../utils/jwt.hpp"
using json = nlohmann::json;

class UserController
{
private:
    UserService &service;

public:
    UserController(UserService &srv) : service(srv) {}

    void getUsers(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            auto users = service.getUsers();
            res.status = 200;
            res.set_content(json(users).dump(), "application/json");
        }
        catch (...)
        {
            res.status = 500;
            res.set_content("Server error", "text/plain");
        }
    }

    void getMe(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            std::string userId = req.get_header_value("userId"); // <<<<<< from middleware

            auto user = service.getOne(bsoncxx::oid(userId));

            if (user.is_null())
            {
                res.status = 404;
                res.set_content("User not found", "text/plain");
                return;
            }

            res.status = 200;
            res.set_content(user.dump(), "application/json");
        }
        catch (...)
        {
            res.status = 500;
            res.set_content("Error", "text/plain");
        }
    }

    void updateProfile(const httplib::Request &req, httplib::Response &res)
    {
        json body;
        try
        {
            body = json::parse(req.body);
        }
        catch (...)
        {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
            return;
        }

        try
        {
            std::string userId = req.get_header_value("userId"); // <<<<<< from middleware

            bool ok = service.updateProfile(userId, body);

            if (ok)
            {
                res.status = 200;
                res.set_content("Updated successfully", "text/plain");
            }
            else
            {
                res.status = 404;
                res.set_content("User not found", "text/plain");
            }
        }
        catch (...)
        {
            res.status = 500;
            res.set_content("Error", "text/plain");
        }
    }
};
