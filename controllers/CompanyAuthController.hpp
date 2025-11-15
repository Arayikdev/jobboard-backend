#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include "../services/CompanyAuthService.hpp"

class CompanyAuthController{
    CompanyAuthService service;
public:
    CompanyAuthController(CompanyAuthService srv) : service(srv) {}
    void registerCompany(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            auto body = json::parse(req.body);
            auto email = body["email"].get<std::string>();
            auto password = body["password"].get<std::string>();
            if (!body.contains("email") || !body.contains("password"))
            {
                res.status = 400;
                res.set_content("Missing data", "text/plain");
                return;
            }
            if (service.registerCompany(email, password))
            {
                res.status = 200;
                res.set_content("Company added", "text/plain");
            }
            else
            {
                res.status = 400;
                res.set_content("Company exist", "text/plain");
            }
        }
        catch (const std::exception &e)
        {
            res.status = 400;
            res.set_content("Something went wrong", "text/plain");
        }
    }
};