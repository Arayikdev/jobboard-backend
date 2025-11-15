#include "../third_party/httplib.h"
#include "../controllers/UserAuthController.hpp"
#include "../controllers/CompanyAuthController.hpp"
void registerUserAuthRoutes(httplib::Server &server, UserAuthController &userauthcontroller)
{
    server.Post("/auth/register/user", [&](const httplib::Request &req, httplib::Response &res)
                {   res.set_header("Access-Control-Allow-Origin", "*");
                    userauthcontroller.registerUser(req, res); });
}

void registerCompanyAuthRoutes(httplib::Server &server, CompanyAuthController &companyauthcontroller)
{
    server.Post("/auth/register/company", [&](const httplib::Request &req, httplib::Response &res)
                {   res.set_header("Access-Control-Allow-Origin", "*");
                    companyauthcontroller.registerCompany(req, res); });
}