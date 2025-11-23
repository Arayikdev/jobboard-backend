#pragma once

#include "../models/Job.hpp"
#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <vector>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <bsoncxx/oid.hpp>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/sub_array.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/basic/document.hpp>

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
                filter.append(
                    kvp("skills",
                        make_document(kvp("$all", skills_arr))));
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
                                   { return c == '[' || c == ']' || c == ' ' || c == '"'; }),
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
                    filter.append(kvp("salaryRange.min", make_document(kvp("$lte", qMax))));
                    filter.append(kvp("salaryRange.max", make_document(kvp("$gte", qMin))));
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
        auto maybe_job = collection.find_one(
            bsoncxx::builder::stream::document{}
            << "_id" << job_oid
            << "status" << "approved"
            << bsoncxx::builder::stream::finalize);

        if (!maybe_job)
            return json::object();

        return json::parse(bsoncxx::to_json(maybe_job->view()));
    }

    // ---------------- UPDATE JOB ----------------
    json updateJob(const bsoncxx::oid &jobId,
                   const std::string &companyId,
                   const json &body)
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        // 1. Найти job в базе
        std::cout << "before search" << std::endl;
        auto jobOpt = collection.find_one(
            make_document(kvp("_id", jobId)));

        if (!jobOpt)
        {
            return {{"error", "Job not found"}};
        }

        json jobJson = json::parse(bsoncxx::to_json(*jobOpt));

        // 2. Проверить принадлежность компании
        std::cout << "before checking" << std::endl;
        if (jobJson["companyId"].get<std::string>() != companyId)
        {
            return {
                {"error", "Forbidden: cannot update job of another company"},
                {"status", 403}};
        }

        // 3. Собрать $set документ
        bsoncxx::builder::basic::document set_doc{};
        std::cout << "before cycle" << std::endl;

        for (auto it = body.begin(); it != body.end(); ++it)
        {
            const std::string &key = it.key();

            if (key != "_id" && key != "createdAt" && key != "companyId")
            {
                append_json_value(set_doc, key, it.value());
            }
        }

        // 4. Обновить updatedAt
        std::cout << "before update one" << std::endl;

        set_doc.append(kvp("updatedAt", bsoncxx::types::b_date{
                                            std::chrono::system_clock::now()}));

        bsoncxx::builder::basic::document update_doc{};
        update_doc.append(kvp("$set", set_doc.extract()));
        std::cout << "after inserting" << std::endl;

        auto result = collection.update_one(
            make_document(kvp("_id", jobId)),
            update_doc.view());
        std::cout << "after funtion call" << std::endl;

        if (!result || result->modified_count() == 0)
        {
            std::cout << "in if" << std::endl;
            return {{"error", "Update failed"}};
        }
        std::cout << "not if" << std::endl;

        return {{"message", "Job updated"}};
    }

    // ----------- JSON → BSON helper -------------
    static void append_json_value(
        bsoncxx::builder::basic::document &doc,
        const std::string &key,
        const json &value)
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
            doc.append(kvp(key,
                           [&](sub_array sub)
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

    // ---------------- CREATE JOB ----------------
    json createJob(const std::string &companyId, const json &body)
    {
        try
        {
            std::cout << "before object" << std::endl;
            Job job(companyId, body);
            std::cout << "after object" << std::endl;

            auto result = collection.insert_one(job.toBson().view());

            if (!result)
            {
                std::cout << "in service if" << std::endl;
                return {{"error", "Failed to create job"}};
            }
            return {
                {"_id", result->inserted_id().get_oid().value.to_string()},
                {"message", "Job created"}};
        }
        catch (const std::exception &e)
        {
            std::cout << "in catch serce" << std::endl;
            return {{"error", e.what()}};
        }
    }

    // ---------------- DELETE JOB ----------------
    json deleteJob(const bsoncxx::oid &jobId,
                   const std::string &companyId)
    {
        // 1. find job
        auto job = collection.find_one(
            make_document(
                kvp("_id", jobId)));

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
        auto result = collection.delete_one(
            make_document(
                kvp("_id", jobId)));

        if (!result || result->deleted_count() == 0)
        {
            return {{"error", "Delete failed"}};
        }

        return {{"message", "Job deleted"}};
    }
};
