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

    void getJobByObjectId(const bsoncxx::oid &jobId, httplib::Response &res)
    {
        try
        {
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

    void createJob(const std::string &ownerId, const httplib::Request &req, httplib::Response &res)
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
            json created = service.createJob(ownerId, body);

            if (created.empty())
            {
                res.status = 400;
                res.set_content("Failed to create job", "text/plain");
            }
            else
            {
                res.status = 201;
                res.set_content(created.dump(), "application/json");
            }
        }
        catch (const std::exception &e)
        {
            res.status = 400;
            res.set_content("Wrong Data", "text/plain");
        }
    }

    void updateJob(std::string id, const httplib::Request &req, httplib::Response &res)
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
            bool updated = service.updateJob(id, body);

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

    void deleteJob(std::string id, httplib::Response &res)
    {
        try
        {
            bool removed = service.deleteJob(id);

            if (removed)
            {
                res.status = 200;
                res.set_content("Job deleted successfully", "text/plain");
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
};
