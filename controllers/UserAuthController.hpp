#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include "../services/UserAuthService.hpp"
using json = nlohmann::json;
class UserAuthController
{
    UserAuthService service;
public:
    UserAuthController(UserAuthService srv) : service(srv) {}
    void registerUser(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            auto body = json::parse(req.body);
            auto email = body["email"].get<std::string>();
            auto password = body["password"].get<std::string>();
            if (!body.contains("email") || !body.contains("password"))
            {
                std::cout << "empty" << std::endl;
                res.status = 400;
                res.set_content("Missing data", "text/plain");
                return;
            }
            if (service.registerUser(email, password))
            {
                std::cout << "in if" << std::endl;
                res.status = 200;
                res.set_content("User added", "text/plain");
            }
            else
            {
                std::cout << "in else" << std::endl;
                res.status = 400;
                res.set_content("User exist", "text/plain");
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "in catch" << std::endl;
            res.status = 400;
            res.set_content("Something went wrong", "text/plain");
        }
    }
};