#ifndef JOB_HPP
#define JOB_HPP

#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include "../third_party/json.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

using json = nlohmann::json;

class Job
{
private:
    std::string companyId;
    std::string title;
    std::string description;
    std::vector<std::string> requiredLanguages;
    std::string grade;
    std::vector<std::string> skills;
    std::vector<std::string> category;
    struct SalaryRange
    {
        double min;
        double max;
    } salaryRange;
    std::string location;
    std::vector<std::string> workType;
    std::string status;               
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;

    bool validate_job_json(const json &j)
    {
        static const std::vector<std::string> required_fields = {
            "companyId", "title", "description", "requiredLanguages",
            "grade", "skills", "category", "salaryRange",
            "location", "workType", "status"};

        for (const auto &field : required_fields)
        {
            if (!j.contains(field))
                return false;
        }

        if (!j["salaryRange"].contains("min") || !j["salaryRange"].contains("max"))
            return false;

        return true;
    }

public:
    Job(const json &j)
    {
        if (!validate_job_json(j))
            throw std::runtime_error("Missing required job field(s)");

        companyId = j["companyId"].get<std::string>();
        title = j["title"].get<std::string>();
        description = j["description"].get<std::string>();
        requiredLanguages = j["requiredLanguages"].get<std::vector<std::string>>();
        grade = j["grade"].get<std::string>();
        skills = j["skills"].get<std::vector<std::string>>();
        category = j["category"].get<std::vector<std::string>>();
        salaryRange.min = j["salaryRange"]["min"].get<double>();
        salaryRange.max = j["salaryRange"]["max"].get<double>();
        location = j["location"].get<std::string>();
        workType = j["workType"].get<std::vector<std::string>>();
        status = j["status"].get<std::string>();

        createdAt = std::chrono::system_clock::now();
        updatedAt = createdAt;
    }

    bsoncxx::document::value toBson() const
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;
        using bsoncxx::builder::basic::sub_array;
        using bsoncxx::types::b_date;

        bsoncxx::builder::basic::array requiredLangsArr;
        for (const auto &lang : requiredLanguages)
            requiredLangsArr.append(lang);

        bsoncxx::builder::basic::array skillsArr;
        for (const auto &skill : skills)
            skillsArr.append(skill);

        bsoncxx::builder::basic::array categoryArr;
        for (const auto &cat : category)
            categoryArr.append(cat);

        bsoncxx::builder::basic::array workTypeArr;
        for (const auto &type : workType)
            workTypeArr.append(type);

        return make_document(
            kvp("companyId", companyId),
            kvp("title", title),
            kvp("description", description),
            kvp("requiredLanguages", requiredLangsArr),
            kvp("grade", grade),
            kvp("skills", skillsArr),
            kvp("category", categoryArr),
            kvp("salaryRange", make_document(
                                   kvp("min", salaryRange.min),
                                   kvp("max", salaryRange.max))),
            kvp("location", location),
            kvp("workType", workTypeArr),
            kvp("status", status),
            kvp("createdAt", bsoncxx::types::b_date(std::chrono::system_clock::now())),
            kvp("updatedAt", bsoncxx::types::b_date(std::chrono::system_clock::now())));
    }

    json toJson() const
    {
        return json{
            {"companyId", companyId},
            {"title", title},
            {"description", description},
            {"requiredLanguages", requiredLanguages},
            {"grade", grade},
            {"skills", skills},
            {"category", category},
            {"salaryRange", {{"min", salaryRange.min}, {"max", salaryRange.max}}},
            {"location", location},
            {"workType", workType},
            {"status", status},
            {"createdAt", std::chrono::duration_cast<std::chrono::milliseconds>(
                              createdAt.time_since_epoch())
                              .count()},
            {"updatedAt", std::chrono::duration_cast<std::chrono::milliseconds>(
                              updatedAt.time_since_epoch())
                              .count()}};
    }
};

#endif
