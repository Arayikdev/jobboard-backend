#include "../utils/jwt.hpp"
#include "../utils/Password.hpp"
#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using json = nlohmann::json;

class AuthService
{
    mongocxx::collection users;
    mongocxx::collection companies;

public:
    AuthService(mongocxx::collection cmp, mongocxx::collection u)
        : users(u), companies(cmp) {}

    json sanitize(const bsoncxx::document::view &view)
    {
        json obj = json::parse(bsoncxx::to_json(view));

        // removing password and isPublic filds
        obj.erase("password");
        obj.erase("isPublic"); 

        return obj;
    }

    json login(const std::string &email, const std::string &password)
    {
        bsoncxx::builder::stream::document filter{};
        filter << "email" << email;

        // ===== USER =====
        auto user = users.find_one(filter.view());
        if (user)
        {
            auto view = user->view();
            std::string storedHash = std::string(view["password"].get_string().value);

            if (!checkPassword(password, storedHash))
                return {{"error", "Invalid password"}};

            std::string userId = view["_id"].get_oid().value.to_string();
            std::string token = JWT::createToken(userId, "user");

            json cleanUser = sanitize(view);

            return {
                {"role" , "user"},
                {"token", token},
                {"user", cleanUser}};
        }

        auto company = companies.find_one(filter.view());
        if (company)
        {
            auto view = company->view();
            std::string storedHash = std::string(view["password"].get_string().value);

            if (!checkPassword(password, storedHash))
                return {{"error", "Invalid password"}};

            std::string companyId = view["_id"].get_oid().value.to_string();
            std::string token = JWT::createToken(companyId, "company");

            json cleanCompany = sanitize(view);

            return {
                {"role" , "company"},
                {"token", token},
                {"company", cleanCompany}};
        }

        return {{"error", "User not found"}};
    }
};
