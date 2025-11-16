#include "../controllers/UserController.hpp"
void registerUserRoutes(httplib::Server &server, UserController &usercontroller)
{
  server.Get("/users", [&](const httplib::Request &req, httplib::Response &res)
             {
              res.set_header("Access-Control-Allow-Origin", "*"); 
              usercontroller.getUsers(req, res); });

  server.Get(R"(/users/([a-fA-F0-9]{24}))", [&](const httplib::Request &req, httplib::Response &res)
             {
      res.set_header("Access-Control-Allow-Origin", "*");    // Converting sub_match to std::string 
    usercontroller.getMe(req, res); });

  server.Put(R"(/users/([a-fA-F0-9]{24}))", [&](const httplib::Request &req, httplib::Response &res)
             {
              res.set_header("Access-Control-Allow-Origin", "*");
    usercontroller.updateProfile(req, res); });
}