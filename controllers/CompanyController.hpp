#pragma once

#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <vector>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include "../services/CompanyService.hpp"

using json = nlohmann::json;

class CompanyController
{
    CompanyService &service;

public:
    CompanyController(CompanyService &srv) : service(srv) {}
    void getAllJobs(const bsoncxx::oid &companyId, httplib::Response &res)
    {
        auto jobs = service.getAllJobs(companyId);
        std::cout << jobs.size() << std::endl;
        res.status = 200;
        res.set_content(json(jobs).dump(), "application/json");
    }
    void getCompanyPublicProfile(const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            std::string companyIdStr = req.matches[1].str();
            bsoncxx::oid companyId(companyIdStr);

            json company = service.getPublicCompany(companyId);

            if (company.is_null())
            {
                res.status = 404;
                res.set_content("Company not found", "text/plain");
                return;
            }

            res.status = 200;
            res.set_content(company.dump(), "application/json");
        }
        catch (...)
        {
            res.status = 400;
            res.set_content("Invalid company id", "text/plain");
        }
    }
    void updateCompanyProfile(const httplib::Request &req,
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

        try
        {
            bsoncxx::oid companyOid(companyId);

            json result = service.updateCompanyProfile(companyOid, body);

            if (result.contains("error"))
            {
                res.status = result.value("status", 400);
                res.set_content(result.dump(), "application/json");
                return;
            }

            res.status = 200;
            res.set_content(result.dump(), "application/json");
        }
        catch (...)
        {
            res.status = 400;
            res.set_content("Invalid Company ID", "text/plain");
        }
    }
};