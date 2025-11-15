#pragma once

#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <vector>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/collection.hpp>

using json = nlohmann::json;
using namespace bsoncxx::builder::basic;

class JobService
{
private:
    mongocxx::collection collection;

public:
    JobService(mongocxx::collection coll) : collection(coll) {}

    std::vector<json> getJobs(const httplib::Request &, httplib::Response &)
    {
        std::vector<json> jobs;

        for (auto &&doc : collection.find({}))
        {
            auto tmp = json::parse(bsoncxx::to_json(doc));

            if (tmp.value("status", "pending") == "approved")
                jobs.push_back(tmp);
        }

        return jobs;
    }

    json getOne(const bsoncxx::oid &job_oid)
    {
        auto maybe_job = collection.find_one(
            bsoncxx::builder::stream::document{}
            << "_id" << job_oid
            << "status" << "approved"
            << bsoncxx::builder::stream::finalize
        );

        if (!maybe_job)
            return json::object();

        return json::parse(bsoncxx::to_json(maybe_job->view()));
    }

    bool updateJob(const std::string &id, const json &body)
    {
        bsoncxx::oid job_oid;

        try
        {
            job_oid = bsoncxx::oid{id};
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

        set_doc.append(kvp(
            "updatedAt",
            bsoncxx::types::b_date{std::chrono::system_clock::now()}
        ));

        document update_doc{};
        update_doc.append(kvp("$set", set_doc.extract()));

        auto filter = make_document(kvp("_id", job_oid));
        auto result = collection.update_one(filter.view(), update_doc.view());

        return result && result->modified_count() > 0;
    }

    static void append_json_value(document &doc,
                                  const std::string &key,
                                  const json &value)
    {
        if (value.is_null())
        {
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
                }
            }));
        }
        else
        {
            doc.append(kvp(key, value.dump()));
        }
    }

    json createJob(const std::string &ownerId, const json &body)
    {
        document doc{};

        for (auto it = body.begin(); it != body.end(); ++it)
        {
            const std::string &key = it.key();
            if (key != "_id" && key != "createdAt" && key != "updatedAt")
                append_json_value(doc, key, it.value());
        }

        doc.append(kvp("ownerId", ownerId));
        doc.append(kvp("status", "pending"));
        doc.append(kvp("createdAt", bsoncxx::types::b_date{
                                   std::chrono::system_clock::now()}));
        doc.append(kvp("updatedAt", bsoncxx::types::b_date{
                                   std::chrono::system_clock::now()}));

        bsoncxx::document::value full_doc = doc.extract();
        auto result = collection.insert_one(full_doc.view());

        if (!result)
            return json::object();

        return json::parse(bsoncxx::to_json(full_doc.view()));
    }

    bool deleteJob(const std::string &id)
    {
        bsoncxx::oid job_oid;
        try
        {
            job_oid = bsoncxx::oid{id};
        }
        catch (const std::exception &)
        {
            return false; 
        }

        auto filter = make_document(kvp("_id", job_oid));
        auto result = collection.delete_one(filter.view());

        return result && result->deleted_count() > 0;
    }

};
