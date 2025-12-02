#include "config/Database.hpp"
#include "models/Company.hpp"
#include "routes/AdminRoutes.hpp"
#include "routes/ApplicationRoutes.hpp"
#include "routes/AuthRoutes.hpp"
#include "routes/CompanyRoutes.hpp"
#include "routes/JobRoutes.hpp"
#include "routes/UserRoutes.hpp"
#include "third_party/httplib.h"
#include "utils/jwt.hpp"

int main() {
  // Initializing Database
  Env::load();
  Database database;
  auto db = database.getDb();

  auto usersCollection = database.getCollection("users");
  auto emailCollection = database.getCollection("email");
  auto companyCollection = database.getCollection("company");
  auto jobCollection = database.getCollection("jobs");
  auto applicationCollection = database.getCollection("applications");

  httplib::Server server;
  server.Options(".*",
                 [&](const httplib::Request &req, httplib::Response &res) {
                   res.set_header("Access-Control-Allow-Origin", "*");
                   res.set_header("Access-Control-Allow-Methods",
                                  "GET, POST, PUT, DELETE, PATCH, OPTIONS");
                   res.set_header("Access-Control-Allow-Headers",
                                  "Content-Type, Authorization");
                   res.status = 200;
                 });
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

  CompanyService companyservice(companyCollection, jobCollection);
  CompanyController companycontroller(companyservice);
  ApplicationService applicationservice(applicationCollection);

  ApplicationController applicationController(applicationservice, jobservice);

  AdminService adminservice(jobCollection, applicationCollection);
  AdminController admincontroller(adminservice);

  registerApplicationRoutes(server, applicationController);

  registerUserRoutes(server, userController);
  registerUserAuthRoutes(server, userauthcontroller);
  registerCompanyAuthRoutes(server, companyauthcontroller);
  login(server, authcontroller);
  registerJobRoutes(server, jobcontroller);
  registerCompanyRoutes(server, companycontroller);
  registerAdminRoutes(server, admincontroller);

  std::cout << "🚀 Server running on http://0.0.0.0:8080\n";
  server.listen("0.0.0.0", 8080);
}
