#include "../controllers/UserController.hpp"
void registerUserRoutes(httplib::Server &server, UserController &usercontroller)
{
    server.Get("/users", [&](const httplib::Request &req, httplib::Response &res)
               { usercontroller.getUsers(req, res); 
                 std::cout << "get" << std::endl; });

    server.Get(R"(/users/([a-fA-F0-9]{24}))", [&](const httplib::Request &req, httplib::Response &res)
               {
    // преобразуем sub_match в std::string
    std::string idStr = req.matches[1].str();
    bsoncxx::oid userId(idStr);  // теперь ObjectId корректно создается
    usercontroller.getUserByObjectId(userId, res); });

    server.Put(R"(/users/([a-fA-F0-9]{24}))", [&](const httplib::Request &req, httplib::Response &res)
               {
         std::string idStr = req.matches[1].str();
    bsoncxx::oid userId(idStr);  // теперь ObjectId корректно создается
    usercontroller.updateUserProfile(idStr, req, res); });
}