#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include "../services/AuthService.hpp"

class AuthController
{
    AuthService service;

public:
    AuthController(AuthService srv) : service(srv) {}

    void login(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            json body = json::parse(req.body);

            if (!body.contains("email") || !body.contains("password"))
            {
                res.status = 400;
                res.set_content(
                    R"({"error":"Missing email or password"})",
                    "application/json"
                );
                return;
            }

            std::string email = body["email"].get<std::string>();
            std::string password = body["password"].get<std::string>();

            json result = service.login(email, password);

            if (result.contains("error"))
            {
                res.status = 400;
                res.set_content(result.dump(), "application/json");
                return;
            }

            // success
            res.status = 200;
            res.set_content(result.dump(), "application/json");
        }
        catch (const std::exception &e)
        {
            json err = {{"error", e.what()}};
            res.status = 500;
            res.set_content(err.dump(), "application/json");
        }
        catch (...)
        {
            res.status = 500;
            res.set_content(R"({"error":"Internal server error"})", "application/json");
        }
    }
};
