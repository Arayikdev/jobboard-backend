#ifndef COMPANY_HPP
#define COMPANY_HPP

#include <vector>
#include <chrono>
#include "../third_party/json.hpp"
#include <string>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

class Company
{
private:
    std::string name;
    std::string email;
    std::string website;
    std::string description;
    std::string avatarBase64;
    std::string location;
    std::chrono::system_clock::time_point created_at;

    bool validate_company_json(const json &j)
    {
        static const std::vector<std::string> required_fields = {
            "name", "email", "website", "description", "avatarBase64", "location"};

        for (const auto &field : required_fields)
        {
            if (!j.contains(field))
                return false;
        }
        return true;
    }

public:
    Company(const json &j)
    {
        if (!validate_company_json(j))
        {
            throw std::runtime_error("Missing required company field");
        }

        created_at = std::chrono::system_clock::now();
        name = j["name"].get<std::string>();
        email = j["email"].get<std::string>();
        website = j["website"].get<std::string>();
        description = j["description"].get<std::string>();
        avatarBase64 = j["avatarBase64"].get<std::string>();
        location = j["location"].get<std::string>();
    }

    bsoncxx::document::value toBson() const
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        return make_document(
            kvp("name", name),
            kvp("email", email),
            kvp("website", website),
            kvp("description", description),
            kvp("avatarBase64", avatarBase64),
            kvp("location", location),
            kvp("createdAt", bsoncxx::types::b_date(created_at)));
    }

    json toJson() const
    {
        return json{
            {"name", name},
            {"email", email},
            {"website", website},
            {"description", description},
            {"avatarBase64", avatarBase64},
            {"location", location},
            {"created_at", std::chrono::duration_cast<std::chrono::milliseconds>(
                               created_at.time_since_epoch())
                               .count()}};
    }
};

#endif
