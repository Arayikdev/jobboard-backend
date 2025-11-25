#pragma once
#include "../controllers/UserController.hpp"
#include "../middleware/AuthMiddleware.hpp"

void registerUserRoutes(httplib::Server &server,
                        UserController &usercontroller) {
  // GET /users
  server.Get("/users",
             [&](const httplib::Request &req, httplib::Response &res) {
               res.set_header("Access-Control-Allow-Origin", "*");
               usercontroller.getUsers(req, res);
             });

  // GET /users/:id
  server.Get(R"(/users/([a-fA-F0-9]{24}))", [&](const httplib::Request &req,
                                                httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");

    std::string tokenUserId;
    bool loggedIn = AuthMiddleware::verifyUser(req, res, tokenUserId);

    usercontroller.getUserById(req, res, tokenUserId, loggedIn);
  });

  // PUT /users/:id
  server.Put(R"(/users/([a-fA-F0-9]{24}))", [&](const httplib::Request &req,
                                                httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");

    std::string tokenUserId;
    bool loggedIn = AuthMiddleware::verifyUser(req, res, tokenUserId);

    if (!loggedIn) {
      res.status = 401;
      res.set_content("Unauthorized", "text/plain");
      return;
    }
    usercontroller.updateProfile(req, res, tokenUserId);
  });
}
