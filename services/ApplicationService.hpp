#pragma once

#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include "../models/Application.hpp"
#include "../third_party/json.hpp"

using json = nlohmann::json;

class ApplicationService {
private:
    mongocxx::collection collection;

public:
    ApplicationService(mongocxx::collection coll)
        : collection(coll) {}

    // CREATE APPLICATION
    json createApplication(const json &appJson)
    {
        Application app(appJson);
        auto bson_value = app.toBson();
        auto result = collection.insert_one(bson_value.view());

        if (!result)
            throw std::runtime_error("insert failed");

        json saved = app.toJson();
        saved["_id"] = result->inserted_id().get_oid().value.to_string();
        return saved;
    }

    // GET USER'S APPLICATIONS
    std::vector<json> getApplicationsByUser(const std::string &userId)
    {
        bsoncxx::builder::basic::document filter{};
        filter.append(bsoncxx::builder::basic::kvp("userId", userId));

        std::vector<json> list;
        for (auto &doc : collection.find(filter.view())) {
            list.push_back(json::parse(bsoncxx::to_json(doc)));
        }
        return list;
    }

    // GET APPLICATIONS BY JOB
    std::vector<json> getApplicationsByJob(const std::string &jobId)
    {
        bsoncxx::builder::basic::document filter{};
        filter.append(bsoncxx::builder::basic::kvp("jobId", jobId));

        std::vector<json> list;
        for (auto &doc : collection.find(filter.view())) {
            list.push_back(json::parse(bsoncxx::to_json(doc)));
        }
        return list;
    }

    // GET APPLICATIONS BY COMPANY
    std::vector<json> getApplicationsByCompany(const std::string &companyId)
    {
        bsoncxx::builder::basic::document filter{};
        filter.append(bsoncxx::builder::basic::kvp("companyId", companyId));

        std::vector<json> list;
        for (auto &doc : collection.find(filter.view())) {
            list.push_back(json::parse(bsoncxx::to_json(doc)));
        }
        return list;
    }
};
