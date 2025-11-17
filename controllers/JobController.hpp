#pragma once

#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <vector>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include "../services/JobService.hpp"

using json = nlohmann::json;

class JobController
{
private:
    JobService &service;

public:
    JobController(JobService &srv) : service(srv) {}

    void getJobs(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            auto jobs = service.getJobs(req, res);
            res.status = 200;
            res.set_content(json(jobs).dump(), "application/json");
        }
        catch (const std::exception &e)
        {
            res.status = 400;
            res.set_content("Wrong Data", "text/plain");
        }
    }

    void getJobByObjectId(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            std::string idStr = req.matches[1].str();
            bsoncxx::oid jobId(idStr);
            auto job = service.getOne(jobId);

            if (job.is_null())
            {
                res.status = 404;
                res.set_content("Job not found", "text/plain");
            }
            else
            {
                res.status = 200;
                res.set_content(job.dump(), "application/json");
            }
        }
        catch (const std::exception &e)
        {
            res.status = 400;
            res.set_content("Wrong Data", "text/plain");
        }
    }

    void createJob(const httplib::Request &req,
                   httplib::Response &res,
                   const std::string &companyId)
    {
        json body;

        try
        {
            body = json::parse(req.body);
        }
        catch (...)
        {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
            return;
        }

        auto result = service.createJob(companyId, body);

        if (result.contains("error"))
        {
            res.status = 400;
            res.set_content(result.dump(), "application/json");
        }
        else
        {
            res.status = 201;
            res.set_content(result.dump(), "application/json");
        }
    }

    void updateJob(const httplib::Request &req, httplib::Response &res, std::string companyId)
    {
        json body;

        try
        {
            body = json::parse(req.body);
        }
        catch (const json::parse_error &e)
        {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
            return;
        }

        try
        {
            std::string jobIdStr = req.matches[1].str();
            bsoncxx::oid jobId(jobIdStr);
            bool updated = service.updateJob(jobId, companyId, body);
            if (updated)
            {
                res.status = 200;
                res.set_content("Job updated successfully", "text/plain");
            }
            else
            {
                res.status = 404;
                res.set_content("Job not found", "text/plain");
            }
        }
        catch (const std::exception &e)
        {
            res.status = 400;
            res.set_content("Wrong Data", "text/plain");
        }
    }

    void deleteJob(const std::string &companyId,
                   const httplib::Request &req,
                   httplib::Response &res)
    {
        try
        {
            std::string jobIdStr = req.matches[1].str();
            bsoncxx::oid jobId(jobIdStr);

            auto result = service.deleteJob(jobId, companyId);

            res.set_content(result.dump(), "application/json");
        }
        catch (...)
        {
            res.status = 400;
            res.set_content("{\"error\": \"Invalid ID\"}", "application/json");
        }
    }
};
