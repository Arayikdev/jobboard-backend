#pragma once
#include "../controllers/JobController.hpp"

void registerJobRoutes(httplib::Server &server, JobController &jobController)
{

    server.Get("/jobs", [&](const httplib::Request &req, httplib::Response &res)
               {
                   jobController.getJobs(req, res);
               });

    server.Get(R"(/jobs/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   std::string idStr = req.matches[1].str();

                   try
                   {
                       bsoncxx::oid jobId(idStr);
                       jobController.getJobByObjectId(jobId, res);
                   }
                   catch (const std::exception &)
                   {
                       res.status = 400;
                       res.set_content("Invalid ObjectId", "text/plain");
                   }
               });

    server.Post("/jobs",
                [&](const httplib::Request &req, httplib::Response &res)
                {
                    std::string ownerId = "anonymous";
                    jobController.createJob(ownerId, req, res);
                });

    server.Put(R"(/jobs/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   std::string idStr = req.matches[1].str();
                   jobController.updateJob(idStr, req, res);
               });

    server.Delete(R"(/jobs/([a-fA-F0-9]{24}))",
                  [&](const httplib::Request &req, httplib::Response &res)
                  {
                      std::string idStr = req.matches[1].str();
                      jobController.deleteJob(idStr, res);
                  });
}
