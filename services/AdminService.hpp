#pragma once

#include "../third_party/json.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <mongocxx/collection.hpp>
#include <vector>

// Removed global using namespace to prevent pollution

class AdminService {
private:
  mongocxx::collection jobcollection;
  mongocxx::collection applicationcollection;

public:
  AdminService(mongocxx::collection jobcoll, mongocxx::collection applcoll)
      : jobcollection(jobcoll), applicationcollection(applcoll) {}

  // GET /admin/jobs/pending
  std::vector<nlohmann::json> getPendingJobs() {
    using namespace bsoncxx::builder::basic;
    using json = nlohmann::json;
    std::vector<json> pending;

    auto filter = make_document(kvp("status", "pending"));
    auto cursor = jobcollection.find(filter.view());

    for (auto &&doc : cursor) {
      pending.push_back(json::parse(bsoncxx::to_json(doc)));
    }

    return pending;
  }

  std::vector<nlohmann::json> getPendingApplications() {
    using namespace bsoncxx::builder::basic;
    using json = nlohmann::json;
    std::vector<json> pending;

    auto filter = make_document(kvp("status", "pending"));
    auto cursor = applicationcollection.find(filter.view());

    for (auto &&doc : cursor) {
      pending.push_back(json::parse(bsoncxx::to_json(doc)));
    }

    return pending;
  }

  // APPROVE JOB
  bool approveJob(const std::string &id) {
    using namespace bsoncxx::builder::basic;
    bsoncxx::oid job_oid;
    try {
      job_oid = bsoncxx::oid{id};
    } catch (...) {
      return false;
    }

    auto filter = make_document(kvp("_id", job_oid));

    auto update = make_document(
        kvp("$set", make_document(kvp("status", "approved"),
                                  kvp("updatedAt",
                                      bsoncxx::types::b_date{
                                          std::chrono::system_clock::now()}))));

    auto result = jobcollection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
  }

  bool approveApplication(const std::string &id) {
    using namespace bsoncxx::builder::basic;
    bsoncxx::oid app_oid;
    try {
      app_oid = bsoncxx::oid{id};
    } catch (...) {
      return false;
    }

    auto filter = make_document(kvp("_id", app_oid));

    auto update =
        make_document(kvp("$set", make_document(kvp("status", "approved"))));

    auto result =
        applicationcollection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
  }

  bool rejectApplication(const std::string &id) {
    using namespace bsoncxx::builder::basic;
    bsoncxx::oid app_oid;
    try {
      app_oid = bsoncxx::oid{id};
    } catch (...) {
      return false;
    }

    auto filter = make_document(kvp("_id", app_oid));

    auto update =
        make_document(kvp("$set", make_document(kvp("status", "rejected"))));

    auto result =
        applicationcollection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
  }

  // REJECT JOB
  bool rejectJob(const std::string &id) {
    using namespace bsoncxx::builder::basic;
    bsoncxx::oid job_oid;
    try {
      job_oid = bsoncxx::oid{id};
    } catch (...) {
      return false;
    }

    auto filter = make_document(kvp("_id", job_oid));

    auto update = make_document(
        kvp("$set", make_document(kvp("status", "rejected"),
                                  kvp("updatedAt",
                                      bsoncxx::types::b_date{
                                          std::chrono::system_clock::now()}))));

    auto result = jobcollection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
  }
};
