#pragma once

#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include "../services/ApplicationService.hpp"
#include "../services/JobService.hpp"   // для поиска job

using json = nlohmann::json;

class ApplicationController {
private:
    ApplicationService &service;
    JobService &jobService;

public:
    ApplicationController(ApplicationService &s, JobService &j)
        : service(s), jobService(j) {}

    // ---------------------------------------------------------
    // POST /applications — apply to job
    // ---------------------------------------------------------
    
    void createApplication(const httplib::Request &req,
                           httplib::Response &res,
                           const std::string &userId)
    {
        try {
            json body = json::parse(req.body);

            if (!body.contains("jobId") ||
                !body.contains("message") ||
                !body.contains("resumeUrl"))
            {
                res.status = 400;
                res.set_content("{\"error\":\"Missing fields\"}", "application/json");
                return;
            }

            std::string jobId = body["jobId"];

            // Validate jobId
            bsoncxx::oid job_oid;
            try { job_oid = bsoncxx::oid(jobId); }
            catch (...) {
                res.status = 400;
                res.set_content("{\"error\":\"Invalid jobId\"}", "application/json");
                return;
            }

            // Find job
            json job = jobService.getOne(job_oid);
            if (job.is_null()) {
                res.status = 404;
                res.set_content("{\"error\":\"Job not found\"}", "application/json");
                return;
            }

            std::string companyId = job["companyId"];

            // Build application JSON
            json appJson = {
                {"jobId", jobId},
                {"userId", userId},
                {"companyId", companyId},
                {"message", body["message"]},
                {"resumeUrl", body["resumeUrl"]},
                {"status", "pending"}
            };

            json saved = service.createApplication(appJson);

            res.status = 201;
            res.set_content(saved.dump(), "application/json");
        }
        catch (...) {
            res.status = 500;
            res.set_content("{\"error\":\"Internal server error\"}", "application/json");
        }
    }

    // ---------------------------------------------------------
    // GET /applications/user/:userId
    // ---------------------------------------------------------
    void getUserApplications(const httplib::Request &req, httplib::Response &res)
    {
        std::string userId = req.matches[1].str();

        auto list = service.getApplicationsByUser(userId);
        res.set_content(json(list).dump(), "application/json");
    }

    // ---------------------------------------------------------
    // GET /applications/job/:jobId
    // ---------------------------------------------------------
    void getApplicationsByJob(const httplib::Request &req, httplib::Response &res)
    {
        std::string jobId = req.matches[1].str();

        auto list = service.getApplicationsByJob(jobId);
        res.set_content(json(list).dump(), "application/json");
    }

    // ---------------------------------------------------------
    // GET /applications/company/:companyId
    // ---------------------------------------------------------
    void getApplicationsByCompany(const httplib::Request &req, httplib::Response &res)
    {
        std::string companyId = req.matches[1].str();

        auto list = service.getApplicationsByCompany(companyId);
        res.set_content(json(list).dump(), "application/json");
    }
};
