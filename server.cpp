#include "routes/UserRoutes.hpp"
#include "routes/AuthRoutes.hpp"
#include "models/Company.hpp"
#include <mongocxx/instance.hpp> // required
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/database.hpp>

int main()
{
    // Initializing  Mongo
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = conn["testdb"];
    auto usersCollection = db["users"];
    auto emailCollection = db["email"];

    UserService userService(usersCollection);
    UserController userController(userService);

    UserAuthService userauthservice(usersCollection, emailCollection);
    UserAuthController userauthcontroller(userauthservice);

    CompanyAuthService companyauthservice(usersCollection, emailCollection);
    CompanyAuthController companyauthcontroller(companyauthservice);

    httplib::Server server;
    registerUserRoutes(server, userController);
    registerUserAuthRoutes(server, userauthcontroller);
    registerCompanyAuthRoutes(server, companyauthcontroller);

    server.Options(".*", [&](const httplib::Request &req, httplib::Response &res)
                   {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.status = 200; });


    

    std::cout << "🚀 Server running on http://0.0.0.0:18080\n";
    server.listen("0.0.0.0", 18080);
}