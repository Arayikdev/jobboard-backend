#pragma once

#include "../controllers/AdminController.hpp"
#include "../middleware/AdminMiddleware.hpp"
#include <bsoncxx/oid.hpp>
#include "../third_party/httplib.h"


void registerAdminRoutes(httplib::Server &server, AdminController &adminController)
{
    // GET /admin/jobs/pending
    server.Get("/admin/jobs/pending",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            if (!AdminMiddleware::check(req, res))
                return;

            adminController.getPendingJobs(res);
        });

    // PATCH /admin/jobs/:id/approve
    server.Patch(R"(/admin/jobs/([a-fA-F0-9]{24})/approve)",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            if (!AdminMiddleware::check(req, res))
                return;

            std::string idStr = req.matches[1].str();

            adminController.approveJob(idStr, res);
        });

    // PATCH /admin/jobs/:id/reject
    server.Patch(R"(/admin/jobs/([a-fA-F0-9]{24})/reject)",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            if (!AdminMiddleware::check(req, res))
                return;

            std::string idStr = req.matches[1].str();

            adminController.rejectJob(idStr, res);
        });
}
