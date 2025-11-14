#include "routes/UserRoutes.hpp"
#include "models/Company.hpp"
#include <mongocxx/instance.hpp> // обязательно
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/database.hpp>

int main()
{
    // Инициализация MongoDB драйвера
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = conn["testdb"];
    auto usersCollection = db["users"];

    UserService userService(usersCollection);
    UserController userController(userService);

    httplib::Server server;
    registerUserRoutes(server, userController);

    std::cout << "🚀 Server running on http://127.0.0.1:18080\n";
    server.listen("0.0.0.0", 18080);
}