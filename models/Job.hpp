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
    std::string category;
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

    static void validate(const json &j)
    {
        auto require_string = [&](const std::string &key)
        {
            if (!j.contains(key) || !j[key].is_string())
                throw std::runtime_error("Missing or invalid field: " + key);
        };

        auto require_array = [&](const std::string &key)
        {
            if (!j.contains(key) || !j[key].is_array())
                throw std::runtime_error("Missing or invalid array: " + key);
        };
        std::cout << "before requre" << std::endl;
        require_string("title");
        std::cout << "after title" << std::endl;
        require_string("description");
        std::cout << "after descr" << std::endl;
        require_array("requiredLanguages");
        std::cout << "after LG" << std::endl;
        require_string("grade");
        std::cout << "after grade" << std::endl;
        require_array("skills");
        std::cout << "after skills" << std::endl;
        require_string("location");
        std::cout << "after loca" << std::endl;
        require_string("category");
        std::cout << "after loc" << std::endl;
        require_array("workType");
        std::cout << "after worktype" << std::endl;

        if (!j["salaryRange"].contains("min") ||
            !(j["salaryRange"]["min"].is_number_float() ||
              j["salaryRange"]["min"].is_number_integer()))
        {
            throw std::runtime_error("salaryRange.min is missing or invalid");
        }
        std::cout << "after min" << std::endl;

        if (!j["salaryRange"].contains("max") ||
            !(j["salaryRange"]["max"].is_number_float() ||
              j["salaryRange"]["max"].is_number_integer()))
        {
            throw std::runtime_error("salaryRange.max is missing or invalid");
        }
        std::cout << "after max" << std::endl;

    }

public:
    Job(const std::string &companyId_, const json &j)
    {
        std::cout << "vefore validate" << std::endl;
        validate(j);
        std::cout << "after validate" << std::endl;

        companyId = companyId_;
        title = j["title"].get<std::string>();
        description = j["description"].get<std::string>();
        requiredLanguages = j["requiredLanguages"].get<std::vector<std::string>>();
        grade = j["grade"].get<std::string>();
        skills = j["skills"].get<std::vector<std::string>>();
        category = j["category"].get<std::string>();
        std::cout << "Before function declaration" << std::endl;
        // 🔥 универсальный парсер
        auto parse_number = [&](const json &val, const std::string &field) -> double
        {
            if (val.is_number_float() || val.is_number_integer())
                return val.get<double>();

            if (val.is_string())
            {
                try
                {
                    return std::stod(val.get<std::string>());
                }
                catch (...)
                {
                    throw std::runtime_error("Invalid value for salaryRange." + field);
                }
            }

            throw std::runtime_error("salaryRange." + field + " must be number or numeric string");
        };
        std::cout << "after function declaration" << std::endl;

        salaryRange.min = parse_number(j["salaryRange"]["min"], "min");
        std::cout << "after min" << std::endl;
        salaryRange.max = parse_number(j["salaryRange"]["max"], "max");
        std::cout << "after max" << std::endl;

        location = j["location"].get<std::string>();        
        std::cout << "after location" << std::endl;

        workType = j["workType"].get<std::vector<std::string>>();
        std::cout << "after worktype" << std::endl;
        
        status = "approved";


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
            kvp("category", category),
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

// {
//   "title": "C++ Backend Developer",
//   "description": "Work on high-performance systems.",
//   "requiredLanguages": ["C++", "Python"],
//   "grade": "Junior",
//   "skills": ["STL", "OOP", "Linux"],
//   "category": "Backend",
//   "salaryRange": {
//     "min": 100000,
//     "max": 200000
//   },
//   "location": "Remote",
//   "workType": ["Remote"],
// }
