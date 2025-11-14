#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include "../third_party/json.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

using json = nlohmann::json;

enum class ApplicationStatus
{
    Pending,
    Accepted,
    Rejected,
    Unknown
};

inline std::string status_to_string(ApplicationStatus status)
{
    switch (status)
    {
    case ApplicationStatus::Pending:
        return "pending";
    case ApplicationStatus::Accepted:
        return "accepted";
    case ApplicationStatus::Rejected:
        return "rejected";
    default:
        return "unknown";
    }
}

inline ApplicationStatus string_to_status(const std::string &s)
{
    if (s == "pending")
        return ApplicationStatus::Pending;
    if (s == "accepted")
        return ApplicationStatus::Accepted;
    if (s == "rejected")
        return ApplicationStatus::Rejected;
    return ApplicationStatus::Unknown;
}

class Application
{
private:
    std::string jobId;
    std::string userId;
    std::string companyId;
    std::string message;
    std::string resumeUrl;
    ApplicationStatus status;
    std::chrono::system_clock::time_point createdAt;

    bool validate_application_json(const json &j)
    {
        static const std::vector<std::string> required_fields = {
            "jobId", "userId", "companyId", "message", "resumeUrl", "status"};
        for (const auto &field : required_fields)
            if (!j.contains(field))
                return false;
        return true;
    }

public:
    Application(const json &j)
    {
        if (!validate_application_json(j))
            throw std::runtime_error("missing argument");

        jobId = j["jobId"].get<std::string>();
        userId = j["userId"].get<std::string>();
        companyId = j["companyId"].get<std::string>();
        message = j["message"].get<std::string>();
        resumeUrl = j["resumeUrl"].get<std::string>();
        status = string_to_status(j["status"].get<std::string>());
        createdAt = std::chrono::system_clock::now();
    }

    bsoncxx::document::value toBson() const
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        return make_document(
            kvp("jobId", jobId),
            kvp("userId", userId),
            kvp("companyId", companyId),
            kvp("message", message),
            kvp("resumeUrl", resumeUrl),
            kvp("status", status_to_string(status)),
            kvp("createdAt", bsoncxx::types::b_date(createdAt)));
    }

    json toJson() const
    {
        return json{
            {"jobId", jobId},
            {"userId", userId},
            {"companyId", companyId},
            {"message", message},
            {"resumeUrl", resumeUrl},
            {"status", status_to_string(status)},
            {"createdAt", std::chrono::duration_cast<std::chrono::milliseconds>(createdAt.time_since_epoch()).count()}};
    }
};

#endif
