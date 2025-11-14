#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <exception>

using json = nlohmann::json;

enum class Grade
{
    Junior,
    Intern,
    Unknown
};

Grade string_to_grade(std::string &&s)
{
    if (s == "Intern")
        return Grade::Intern;
    if (s == "Junior")
        return Grade::Junior;
    return Grade::Unknown;
}

std::string grade_to_string(Grade grade)
{
    switch (grade)
    {
    case Grade::Junior:
        return "Junior";
    case Grade::Intern:
        return "Intern";
    }
    return "Unknown";
}
class User
{
private:
    bool validate_user_json(const json &j)
    {
        static const std::vector<std::string> required_fields = {
            "name", "email", "password", "programming_languages", "grade", "location", "bio", "isPublic", "lastname", "skills", "category"};
        for (const auto &field : required_fields)
            if (!j.contains(field))
                return false;
        return true;
    }

    std::string name;
    std::string lastname;
    std::string email;
    std::string password;
    std::vector<std::string> programming_languages;
    std::vector<std::string> skills;
    std::vector<std::string> category;
    Grade grade;
    std::string location;
    std::string bio;
    bool isPublic;
    std::string avatarBase64;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;

public:
    User(const json &j)
    {
        if (!validate_user_json(j))
            throw std::runtime_error("missing argument");

        programming_languages = j["programming_languages"].get<std::vector<std::string>>();
        skills = j["skills"].get<std::vector<std::string>>();
        category = j["category"].get<std::vector<std::string>>();
        grade = string_to_grade(j["grade"].get<std::string>());
        name = j["name"].get<std::string>();
        email = j["email"].get<std::string>();
        password = j["password"].get<std::string>();
        location = j["location"].get<std::string>();
        bio = j["bio"].get<std::string>();
        lastname = j["lastname"].get<std::string>();
        isPublic = j["isPublic"].get<bool>();
        avatarBase64 = j["avatarBase64"].get<std::string>();
        created_at = std::chrono::system_clock::now();
        updated_at = created_at;
    }

    // Convert User to BSON (MongoDB)
    bsoncxx::document::value toBson() const
    {
        using bsoncxx::builder::basic::array;
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        array langs_builder;
        for (const auto &lang : programming_languages)
        {
            langs_builder.append(lang);
        }

        array categories;
        for (const auto &cat : category)
        {
            categories.append(cat);
        }

        array skills_;
        for (const auto &sk : skills)
        {
            skills_.append(sk);
        }

        return make_document(
            kvp("name", name),
            kvp("email", email),
            kvp("password", password),
            kvp("programming_languages", langs_builder.extract()),
            kvp("grade", grade_to_string(grade)),
            kvp("location", location),
            kvp("bio", bio),
            kvp("isPublic", isPublic),
            kvp("skills", skills_),
            kvp("category", categories),
            kvp("avatarBase64", avatarBase64),
            kvp("lastname", lastname),
            kvp("createdAt", bsoncxx::types::b_date(created_at)),
            kvp("updatedAt", bsoncxx::types::b_date(updated_at)));
    }

    // Convert User to JSON string (HTTP response)
    nlohmann::json toJson() const
    {
        return nlohmann::json{
            {"name", name},
            {"email", email},
            {"password", password},
            {"programming_languages", programming_languages},
            {"grade", grade_to_string(grade)},
            {"location", location},
            {"bio", bio},
            {"lastname", lastname},
            {"skills", skills},
            {"category", category},
            {"avatarBase64", avatarBase64},
            {"isPublic", isPublic},
            {"createdAt", std::chrono::duration_cast<std::chrono::milliseconds>(created_at.time_since_epoch()).count()},
            {"updatedAt", std::chrono::duration_cast<std::chrono::milliseconds>(updated_at.time_since_epoch()).count()}};
    }
};

#endif