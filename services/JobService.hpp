#pragma once

#include "../models/Job.hpp"
#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <vector>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/basic/sub_array.hpp>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/collection.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

using json = nlohmann::json;

class JobService
{
private:
  mongocxx::collection collection;

public:
  JobService(mongocxx::collection coll) : collection(coll) {}

  // ---------------- GET ALL APPROVED JOBS ----------------
  std::vector<json> getJobs(const httplib::Request &req, httplib::Response &)
  {
    using bsoncxx::builder::basic::array;
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    bsoncxx::builder::basic::document filter{};

    // Только approved
    filter.append(kvp("status", "approved"));

    // 1) category (строка)
    if (req.has_param("category"))
    {
      std::string cat = req.get_param_value("category");
      if (!cat.empty())
      {
        filter.append(kvp("category", cat));
      }
    }

    // 2) language -> requiredLanguages (массив строк)
    // Mongo умеет искать значение в массиве просто по равенству:
    // { requiredLanguages: "C++" } матчится на ["C++", "Python"]
    if (req.has_param("language"))
    {
      std::string lang = req.get_param_value("language");
      if (!lang.empty())
      {
        filter.append(kvp("requiredLanguages", lang));
      }
    }

    // 3) skills = "Docker,Algorithms"
    // Требуем, чтобы ВСЕ указанные skills были в массиве skills
    if (req.has_param("skills"))
    {
      std::string skillsStr = req.get_param_value("skills");
      std::stringstream ss(skillsStr);
      std::string skill;

      array skills_arr;

      while (std::getline(ss, skill, ','))
      {
        // немного подчистим пробелы вокруг
        auto start = skill.find_first_not_of(" \t\r\n");
        auto end = skill.find_last_not_of(" \t\r\n");
        if (start == std::string::npos)
          continue;
        skill = skill.substr(start, end - start + 1);

        if (!skill.empty())
        {
          skills_arr.append(skill);
        }
      }

      if (!skills_arr.view().empty())
      {
        filter.append(kvp("skills", make_document(kvp("$all", skills_arr))));
      }
    }

    // 4) location
    if (req.has_param("location"))
    {
      std::string loc = req.get_param_value("location");
      if (!loc.empty())
      {
        filter.append(kvp("location", loc));
      }
    }

    // 5) grade
    if (req.has_param("grade"))
    {
      std::string level = req.get_param_value("grade");
      if (!level.empty())
      {
        filter.append(kvp("grade", level));
      }
    }

    // 6) salary = "min,max" c пересечением диапазонов
    if (req.has_param("salary"))
    {
      std::string s = req.get_param_value("salary");

      // Удаляем [], пробелы
      s.erase(std::remove_if(s.begin(), s.end(),
                             [](char c)
                             {
                               return c == '[' || c == ']' || c == ' ' ||
                                      c == '"';
                             }),
              s.end());

      // Меняем "-" на ","
      std::replace(s.begin(), s.end(), '-', ',');

      // Теперь формат 1000,2000
      std::stringstream ss(s);

      std::string minStr, maxStr;
      if (std::getline(ss, minStr, ',') && std::getline(ss, maxStr, ','))
      {
        try
        {
          double qMin = std::stod(minStr);
          double qMax = std::stod(maxStr);

          // Mongo фильтры
          filter.append(
              kvp("salaryRange.min", make_document(kvp("$lte", qMax))));
          filter.append(
              kvp("salaryRange.max", make_document(kvp("$gte", qMin))));
        }
        catch (...)
        {
        }
      }
    }

    // ---- выполняем запрос ----
    std::vector<json> jobs;
    auto cursor = collection.find(filter.view());

    for (auto &&doc : cursor)
    {
      jobs.push_back(json::parse(bsoncxx::to_json(doc)));
    }

    return jobs;
  }

  // ---------------- GET ONE JOB ----------------
  json getOne(const bsoncxx::oid &job_oid)
  {
    auto maybe_job = collection.find_one(bsoncxx::builder::stream::document{}
                                         << "_id" << job_oid
                                         << bsoncxx::builder::stream::finalize);

    if (!maybe_job)
      return json::object();

    return json::parse(bsoncxx::to_json(maybe_job->view()));
  }

  // ---------------- UPDATE JOB ----------------
  json updateJob(const bsoncxx::oid &jobId, const std::string &companyId,
                 const json &body)
  {
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    // 1. Найти job
    auto jobOpt = collection.find_one(make_document(kvp("_id", jobId)));
    if (!jobOpt)
    {
      return {{"error", "Job not found"}, {"status", 404}};
    }

    json jobJson = json::parse(bsoncxx::to_json(*jobOpt));

    // 2. Проверка принадлежности компании
    if (jobJson["companyId"].get<std::string>() != companyId)
    {
      return {{"error", "Forbidden: cannot update job of another company"},
              {"status", 403}};
    }

    // 3. Формируем $set
    bsoncxx::builder::basic::document set_doc{};

    for (auto it = body.begin(); it != body.end(); ++it)
    {
      const std::string &key = it.key();
      std::cout << "[DEBUG] updateJob key: " << key << ", type: "
                << (it.value().is_object()
                        ? "object"
                        : (it.value().is_string() ? "string" : "other"))
                << std::endl;

      if (key == "_id" || key == "companyId" || key == "createdAt")
        continue;

      // -------------------------
      // SPECIAL CASE: salaryRange
      // -------------------------
      if (key == "salaryRange")
      {
        std::cout << "[DEBUG] Handling salaryRange special case" << std::endl;
        const json &raw = it.value();
        json sr;

        // Handle both object and string representations
        if (raw.is_object())
        {
          sr = raw;
        }
        else if (raw.is_string())
        {
          try
          {
            sr = json::parse(raw.get<std::string>());
          }
          catch (...)
          {
            return {{"error", "salaryRange must be object or JSON string"},
                    {"status", 400}};
          }
        }
        else
        {
          return {{"error", "salaryRange must be object or JSON string"},
                  {"status", 400}};
        }

        // Validate that salary range has min and max
        if (!sr.is_object() || !sr.contains("min") || !sr.contains("max"))
        {
          return {{"error", "salaryRange must contain min and max fields"},
                  {"status", 400}};
        }

        // Parse min and max using the same logic as Job constructor
        try
        {
          double minVal = Job::parseSalaryValue(sr["min"], "min");
          double maxVal = Job::parseSalaryValue(sr["max"], "max");

          // Create BSON document for salary range
          set_doc.append(kvp("salaryRange", make_document(kvp("min", minVal),
                                                          kvp("max", maxVal))));
        }
        catch (const std::exception &e)
        {
          return {{"error", e.what()}, {"status", 400}};
        }
        continue;
      }

      // -------------------------
      // DEFAULT CASE: обычные поля
      // -------------------------
      append_json_value(set_doc, key, it.value());
    }

    // 4. updatedAt
    set_doc.append(kvp(
        "updatedAt", bsoncxx::types::b_date{std::chrono::system_clock::now()}));
    set_doc.append(kvp(
        "status", "pending"));

    bsoncxx::builder::basic::document update_doc{};
    update_doc.append(kvp("$set", set_doc.extract()));

    auto result = collection.update_one(make_document(kvp("_id", jobId)),
                                        update_doc.view());

    if (!result || result->modified_count() == 0)
    {
      return {{"error", "Update failed"}, {"status", 500}};
    }

    return {{"message", "Job updated"}};
  }

  // ----------- JSON → BSON helper -------------
  static void append_json_value(bsoncxx::builder::basic::document &doc,
                                const std::string &key, const json &value)
  {
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::sub_array;

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
        for (const auto &el : value) {
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
      doc.append(kvp(key, value.dump()));
    }
  }

  // ---------------- CREATE JOB ----------------
  json createJob(const std::string &companyId, const json &body)
  {
    try
    {
      Job job(companyId, body);

      auto result = collection.insert_one(job.toBson().view());

      if (!result)
      {
        return {{"error", "Failed to create job"}};
      }
      return {{"_id", result->inserted_id().get_oid().value.to_string()},
              {"message", "Job created"}};
    }
    catch (const std::exception &e)
    {
      return {{"error", e.what()}};
    }
  }

  // ---------------- DELETE JOB ----------------
  json deleteJob(const bsoncxx::oid &jobId, const std::string &companyId)
  {
    // 1. find job
    auto job = collection.find_one(make_document(kvp("_id", jobId)));

    if (!job)
    {
      return {{"error", "Job not found"}};
    }

    json j = json::parse(bsoncxx::to_json(*job));

    // 2. check ownership
    if (j["companyId"].get<std::string>() != companyId)
    {
      return {{"error", "Forbidden: not your job"}, {"status", 403}};
    }

    // 3. delete
    auto result = collection.delete_one(make_document(kvp("_id", jobId)));

    if (!result || result->deleted_count() == 0)
    {
      return {{"error", "Delete failed"}};
    }

    return {{"message", "Job deleted"}};
  }

  json admindeleteJob(const bsoncxx::oid &jobId)
  {
    // 1. find job
    auto job = collection.find_one(make_document(kvp("_id", jobId)));

    if (!job)
    {
      return {{"error", "Job not found"}};
    }

    // 2. delete
    auto result = collection.delete_one(make_document(kvp("_id", jobId)));

    if (!result || result->deleted_count() == 0)
    {
      return {{"error", "Delete failed"}};
    }

    return {{"message", "Job deleted"}};
  }
};