#pragma once

#include "../services/ApplicationService.hpp"
#include "../services/JobService.hpp" // для поиска job
#include "../third_party/httplib.h"
#include "../third_party/json.hpp"

using json = nlohmann::json;

class ApplicationController
{
private:
  ApplicationService &service;
  JobService &jobService;

public:
  ApplicationController(ApplicationService &s, JobService &j)
      : service(s), jobService(j) {}

  // ---------------------------------------------------------
  // POST /applications — apply to job
  // ---------------------------------------------------------
  void createApplication(const httplib::Request &req, httplib::Response &res,
                         const std::string &userId)
  {
    try
    {
      json body = json::parse(req.body);

      if (!body.contains("jobId") || !body.contains("message") ||
          !body.contains("resumeUrl"))
      {
        res.status = 400;
        res.set_content("{\"error\":\"Missing fields\"}", "application/json");
        return;
      }

      std::string jobId = body["jobId"];

      // Validate jobId
      bsoncxx::oid job_oid;
      try
      {
        job_oid = bsoncxx::oid(jobId);
      }
      catch (...)
      {
        res.status = 400;
        res.set_content("{\"error\":\"Invalid jobId\"}", "application/json");
        return;
      }

      // Find job
      json job = jobService.getOne(job_oid);
      if (job.is_null())
      {
        res.status = 404;
        res.set_content("{\"error\":\"Job not found\"}", "application/json");
        return;
      }

      // Extract companyId
      if (!job.contains("companyId"))
      {
        res.status = 500;
        res.set_content("{\"error\":\"Job missing companyId\"}", "application/json");
        return;
      }

      std::string companyId = job["companyId"].get<std::string>();

      // Prepare application JSON
      json appJson = {
          {"jobId", jobId},
          {"userId", userId},
          {"companyId", companyId},
          {"message", body["message"]},
          {"resumeUrl", body["resumeUrl"]},
          {"status", "pending"}};

      json saved = service.createApplication(appJson);

      // <- IMPORTANT: check service error!
      if (saved.contains("error"))
      {
        res.status = saved.value("status", 400);
        res.set_content(saved.dump(), "application/json");
        return;
      }

      res.status = 201;
      res.set_content(saved.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
      res.status = 500;
      res.set_content("{\"error\":\"Internal server error\"}", "application/json");
    }
  }

  // ---------------------------------------------------------
  // GET /applications/user/:userId
  // ---------------------------------------------------------
  void getUserApplications(std::string userId, const httplib::Request &req,
                           httplib::Response &res)
  {

    auto list = service.getApplicationsByUser(userId);
    res.set_content(json(list).dump(), "application/json");
  }

  // ---------------------------------------------------------
  // GET /applications/job/:jobId
  // ---------------------------------------------------------
  void getApplicationsByJob(const httplib::Request &req,
                            httplib::Response &res)
  {
    std::string jobId = req.matches[1].str();

    auto list = service.getApplicationsByJob(jobId);
    res.set_content(json(list).dump(), "application/json");
  }

  // ---------------------------------------------------------
  // GET /applications/company/:companyId
  // ---------------------------------------------------------
  void getApplicationsByCompany(const httplib::Request &req,
                                httplib::Response &res)
  {
    std::string companyId = req.matches[1].str();

    auto list = service.getApplicationsByCompany(companyId);
    res.set_content(json(list).dump(), "application/json");
  }
};
