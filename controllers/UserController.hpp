#include <vector>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include "../services/UserService.hpp"
using json = nlohmann::json;
class UserController
{
private:
    UserService &service; // reference to service

public:
    UserController(UserService &srv) : service(srv) {}

    void getUsers(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            auto users = service.getUsers(req, res);
            res.status = 400;
            res.set_content(json(users).dump(), "application/json");
        }
        catch (std::exception &e)
        {
            res.status = 400;
            res.set_content("Wrong Data", "text/plain");
        }
    }

    void getUserByObjectId(const bsoncxx::oid &userId, httplib::Response &res)
    {
        try
        {
            auto user = service.getOne(userId);

            if (user.is_null())
            {
                res.status = 404;
                res.set_content("User not found", "text/plain");
            }
            else
            {
                res.status = 200;
                res.set_content(user.dump(), "application/json");
            }
        }
        catch (std::exception e)
        {
            res.status = 400;
            res.set_content("Wrong Data", "text/plain");
        }
    }

    void updateUserProfile(std::string id, const httplib::Request &req, httplib::Response &res)
    {
        json body;
        try
        {
            body = json::parse(req.body);
        }
        catch (const json::parse_error &e)
        {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
            return;
        }
        try
        {
            bool user = service.updateProfile(id, body);
            if (user)
            {
                res.status = 200;
                res.set_content("User updated successfully", "text/plain");
            }
            else
            {
                res.status = 400;
                res.set_content("User not found", "text/plain");
            }
        }
        catch (std::exception &e)
        {
            res.status = 400;
            res.set_content("Wrong Data", "text/plain");
        }
    }
};