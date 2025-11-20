#pragma once
#include "../third_party/json.hpp"
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/collection.hpp>
#include <chrono>

using namespace bsoncxx::builder::basic;
using json = nlohmann::json;

class UserService
{
private:
    mongocxx::collection collection;

public:
    UserService(mongocxx::collection coll) : collection(coll) {}

    // GET /users → only public users
    std::vector<json> getUsers()
    {
        std::vector<json> users;

        for (auto &&doc : collection.find({}))
        {
            auto tmp = json::parse(bsoncxx::to_json(doc));
            if (tmp.value("isPublic", false))
                users.push_back(tmp);
        }

        return users;
    }

    // PUBLIC PROFILE
    json getOnePublic(const bsoncxx::oid &user_oid)
    {
        auto result = collection.find_one(
            make_document(
                kvp("_id", user_oid),
                kvp("isPublic", true)));

        if (!result)
            return json();

        return json::parse(bsoncxx::to_json(result->view()));
    }

    // PRIVATE PROFILE (OWNER)
    json getOnePrivate(const bsoncxx::oid &user_oid)
    {
        auto result = collection.find_one(
            make_document(
                kvp("_id", user_oid)));

        if (!result)
            return json();

        return json::parse(bsoncxx::to_json(result->view()));
    }

    // UPDATE PROFILE (OWNER ONLY)
    bool updateProfile(const std::string &id, const json &body)
    {
        bsoncxx::oid user_oid;
        try
        {
            user_oid = bsoncxx::oid{id};
        }
        catch (...)
        {
            return false;
        }

        document setDoc{};

        for (auto it = body.begin(); it != body.end(); ++it)
        {
            const auto &key = it.key();
            if (key == "_id" || key == "createdAt")
                continue;

            append_json_value(setDoc, key, it.value());
        }

        setDoc.append(kvp("updatedAt",
                          bsoncxx::types::b_date{std::chrono::system_clock::now()}));

        document updateDoc{};
        updateDoc.append(kvp("$set", setDoc.extract()));

        auto filter = make_document(kvp("_id", user_oid));
        auto result = collection.update_one(filter.view(), updateDoc.view());

        return result && result->modified_count() > 0;
    }

    // JSON → BSON helper
    static void append_json_value(document &doc,
                                  const std::string &key,
                                  const json &value)
    {
        using bsoncxx::builder::basic::sub_array;

        if (value.is_null())
            doc.append(kvp(key, bsoncxx::types::b_null{}));
        else if (value.is_boolean())
            doc.append(kvp(key, value.get<bool>()));
        else if (value.is_number_integer())
            doc.append(kvp(key, static_cast<int64_t>(value.get<int64_t>())));
        else if (value.is_number_unsigned())
            doc.append(kvp(key, static_cast<int64_t>(value.get<uint64_t>())));
        else if (value.is_number_float())
            doc.append(kvp(key, value.get<double>()));
        else if (value.is_string())
            doc.append(kvp(key, value.get<std::string>()));
        else if (value.is_array())
        {
            doc.append(kvp(key, [&](sub_array arr)
                           {
                for (const auto &el : value)
                {
                    if (el.is_null())
                        arr.append(bsoncxx::types::b_null{});
                    else if (el.is_boolean())
                        arr.append(el.get<bool>());
                    else if (el.is_number_integer())
                        arr.append(static_cast<int64_t>(el.get<int64_t>()));
                    else if (el.is_number_unsigned())
                        arr.append(static_cast<int64_t>(el.get<uint64_t>()));
                    else if (el.is_number_float())
                        arr.append(el.get<double>());
                    else if (el.is_string())
                        arr.append(el.get<std::string>());
                    else
                        arr.append(el.dump());
                } }));
        }
        else
            doc.append(kvp(key, value.dump()));
    }
};
