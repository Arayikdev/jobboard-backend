#pragma once
#include "../third_party/httplib.h"
#include "../controllers/JobController.hpp"
#include "../middleware/AuthMiddleware.hpp"

void registerJobRoutes(httplib::Server &server, JobController &jobController)
{

    server.Get("/jobs", [&](const httplib::Request &req, httplib::Response &res)
               {
                   res.set_header("Access-Control-Allow-Origin", "*"); 
                   jobController.getJobs(req, res); });

    server.Get(R"(/jobs/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                
                   res.set_header("Access-Control-Allow-Origin", "*");
                   jobController.getJobByObjectId(req, res);
               });

    server.Post("/jobs", [&](const httplib::Request &req, httplib::Response &res)
                {

                    std::string userId;

                    if (!AuthMiddleware::verifyCompany(req, res, userId)) {
                        return; // middleware already sent error
                    }

                    // Call controller
                    res.set_header("Access-Control-Allow-Origin", "*");
                    jobController.createJob(req, res, userId); });

    server.Put(R"(/jobs/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   std::string companyId;

                   if (!AuthMiddleware::verifyCompany(req, res, companyId))
                   {
                       return; // middleware already sent error
                   }
                   res.set_header("Access-Control-Allow-Origin", "*");
                   jobController.updateJob(req, res, companyId);
               });

    server.Delete(R"(/jobs/([a-fA-F0-9]{24}))",
                  [&](const httplib::Request &req, httplib::Response &res)
                  {
                      std::string companyId;

                      if (!AuthMiddleware::verifyCompany(req, res, companyId))
                      {
                          return;
                      }

                      res.set_header("Access-Control-Allow-Origin", "*");
                      jobController.deleteJob(companyId, req, res);
                  });
}
