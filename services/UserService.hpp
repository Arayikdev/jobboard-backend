#pragma once
#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <vector>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/collection.hpp>

using namespace bsoncxx::builder::basic;
using json = nlohmann::json;

class UserService
{
private:
    mongocxx::collection collection;

public:
    UserService(mongocxx::collection coll) : collection(coll) {}

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

    json getOne(const bsoncxx::oid &user_oid)
    {
        std::cout << "one" << std::endl;
        auto maybe_user = collection.find_one(
            bsoncxx::builder::stream::document{}
            << "_id" << user_oid
            << "isPublic" << true
            << bsoncxx::builder::stream::finalize);

        if (!maybe_user)
            return json::object();

        return json::parse(bsoncxx::to_json(maybe_user->view()));
    }

    bool updateProfile(const std::string &id, const json &body)
    {
        bsoncxx::oid user_oid;
        try
        {
            user_oid = bsoncxx::oid{id};
        }
        catch (const std::exception &)
        {
            return false;
        }

        document set_doc{};
        for (auto it = body.begin(); it != body.end(); ++it)
        {
            const std::string &k = it.key();
            if (k != "_id" && k != "createdAt")
                append_json_value(set_doc, k, it.value());
        }

        set_doc.append(kvp("updatedAt",
                           bsoncxx::types::b_date{std::chrono::system_clock::now()}));

        document update_doc{};
        update_doc.append(kvp("$set", set_doc.extract()));

        auto filter = make_document(kvp("_id", user_oid));
        auto result = collection.update_one(filter.view(), update_doc.view());

        return result && result->modified_count() > 0;
    }

    static void append_json_value(document &doc,
                                  const std::string &key,
                                  const json &value)
    {
        if (value.is_null())
        {
            // correct way to append null
            doc.append(kvp(key, bsoncxx::types::b_null{}));
        }
        else if (value.is_boolean())
        {
            doc.append(kvp(key, value.get<bool>()));
        }
        else if (value.is_number_integer())
        {
            doc.append(kvp(key, static_cast<int64_t>(value.get<int64_t>())));
        }
        else if (value.is_number_unsigned())
        {
            doc.append(kvp(key, static_cast<int64_t>(value.get<uint64_t>())));
        }
        else if (value.is_number_float())
        {
            doc.append(kvp(key, value.get<double>()));
        }
        else if (value.is_string())
        {
            doc.append(kvp(key, value.get<std::string>()));
        }
        else if (value.is_array())
        {
            doc.append(kvp(key, [&](sub_array sub)
                           {
                for (const auto &el : value)
                {
                    if (el.is_null())
                        sub.append(bsoncxx::types::b_null{});
                    else if (el.is_boolean())
                        sub.append(el.get<bool>());
                    else if (el.is_number_integer())
                        sub.append(static_cast<int64_t>(el.get<int64_t>()));
                    else if (el.is_number_unsigned())
                        sub.append(static_cast<int64_t>(el.get<uint64_t>()));
                    else if (el.is_number_float())
                        sub.append(el.get<double>());
                    else if (el.is_string())
                        sub.append(el.get<std::string>());
                    else
                        sub.append(el.dump());
                } }));
        }
        else
        {
            // fallback: store JSON as string
            doc.append(kvp(key, value.dump()));
        }
    }
};
