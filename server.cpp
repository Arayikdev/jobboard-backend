#include "routes/UserRoutes.hpp"
#include "routes/AuthRoutes.hpp"
#include "routes/JobRoutes.hpp"
#include "models/Company.hpp"
#include <mongocxx/instance.hpp> // required
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/database.hpp>
#include "utils/jwt.hpp"

int main()
{
    // Initializing  Mongo
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = conn["testdb"];
    auto usersCollection = db["users"];
    auto emailCollection = db["email"];
    auto companyCollection = db["company"];
    auto jobCollection = db["jobs"];

    UserService userService(usersCollection);
    UserController userController(userService);

    UserAuthService userauthservice(usersCollection, emailCollection);
    UserAuthController userauthcontroller(userauthservice);

    CompanyAuthService companyauthservice(companyCollection, emailCollection);
    CompanyAuthController companyauthcontroller(companyauthservice);

    AuthService authservice(companyCollection, usersCollection);
    AuthController authcontroller(authservice);

    JobService jobservice(jobCollection);
    JobController jobcontroller(jobservice);

    httplib::Server server;
    registerUserRoutes(server, userController);
    registerUserAuthRoutes(server, userauthcontroller);
    registerCompanyAuthRoutes(server, companyauthcontroller);
    login(server, authcontroller);
    registerJobRoutes(server, jobcontroller);


    server.Options(".*", [&](const httplib::Request &req, httplib::Response &res)
                   {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.status = 200; });


    

    std::cout << "🚀 Server running on http://0.0.0.0:8080\n";
    server.listen("0.0.0.0", 8080);
}