#pragma once
#include "../third_party/httplib.h"
#include "../controllers/ApplicationController.hpp"
#include "../middleware/AuthMiddleware.hpp"

// Register all application routes
void registerApplicationRoutes(httplib::Server &server,
                               ApplicationController &controller)
{
    // ============================================================
    // POST /applications — apply to job
    // ============================================================
    server.Post("/applications",
                [&](const httplib::Request &req, httplib::Response &res)
                {
                    res.set_header("Access-Control-Allow-Origin", "*");

                    // JWT must provide userId
                    std::string userIdFromToken;
                    if (!AuthMiddleware::verifyUser(req, res, userIdFromToken))
                        return;

                    controller.createApplication(req, res, userIdFromToken);
                });

    // ============================================================
    // GET /applications/user/:userId
    // ============================================================
    server.Get(R"(/applications/user/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   res.set_header("Access-Control-Allow-Origin", "*");

                   // user can only view their own applications
                   std::string userIdFromToken;
                   if (!AuthMiddleware::verifyUser(req, res, userIdFromToken))
                       return;

                   std::string pathId = req.matches[1].str();

                   if (userIdFromToken != pathId)
                   {
                       res.status = 403;
                       res.set_content("You can view only your own applications", "text/plain");
                       return;
                   }

                   controller.getUserApplications(req, res);
               });

    // ============================================================
    // GET /applications/job/:jobId
    // Used by COMPANY or ADMIN
    // ============================================================
    server.Get(R"(/applications/job/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   res.set_header("Access-Control-Allow-Origin", "*");

                   // company or admin check
                   std::string companyIdFromToken;
                   if (!AuthMiddleware::verifyCompany(req, res, companyIdFromToken))
                       return;

                   std::string jobId = req.matches[1].str();

                   controller.getApplicationsByJob(req, res);
               });

    // ============================================================
    // GET /applications/company/:companyId
    // Only company owners can view their own applicants
    // ============================================================
    server.Get(R"(/applications/company/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   res.set_header("Access-Control-Allow-Origin", "*");

                   std::string companyIdFromToken;
                   if (!AuthMiddleware::verifyCompany(req, res, companyIdFromToken))
                       return;

                   std::string pathCompanyId = req.matches[1].str();

                   if (pathCompanyId != companyIdFromToken)
                   {
                       res.status = 403;
                       res.set_content("Forbidden: cannot view another company's applications",
                                       "text/plain");
                       return;
                   }

                   controller.getApplicationsByCompany(req, res);
               });
}
