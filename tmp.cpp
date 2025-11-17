#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

#include "models/Job.hpp"   // <-- твой Job class

using json = nlohmann::json;

// ------------ RANDOM HELPERS ------------

std::string random_from(const std::vector<std::string>& arr) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, arr.size() - 1);
    return arr[dist(gen)];
}

std::vector<std::string> random_list(const std::vector<std::string>& arr, int count) {
    std::vector<std::string> out;
    for (int i = 0; i < count; i++)
        out.push_back(random_from(arr));
    return out;
}

// ------------ GENERATE ONE JOB JSON ------------

json generateRandomJobJson() {
    static std::vector<std::string> companies = {"comp1", "comp2", "comp3", "comp4", "comp5"};
    static std::vector<std::string> titles = {
        "Junior Backend Developer", "Intern C++ Developer",
        "Junior Frontend React", "Intern QA Engineer",
        "Junior Python Developer", "Node.js Intern"
    };
    static std::vector<std::string> languages = {"C++", "Python", "JavaScript", "Go", "Rust", "Java"};
    static std::vector<std::string> grades = {"Intern", "Junior"};
    static std::vector<std::string> skills = {
        "OOP", "DSA", "MongoDB", "REST API", "Linux", "Docker",
        "Git", "Networking", "Multithreading", "Algorithms"
    };
    static std::vector<std::string> categories = {"Backend", "Frontend", "QA", "Mobile", "ML", "DevOps"};
    static std::vector<std::string> workTypes = {"Remote", "Onsite", "Hybrid"};

    // Salary generation
    double min_salary = 100 + rand() % 400;
    double max_salary = min_salary + (rand() % 500);

    json j = {
        {"companyId", random_from(companies)},
        {"title", random_from(titles)},
        {"description", "Generated job description"},
        {"requiredLanguages", random_list(languages, 2)},
        {"grade", random_from(grades)},
        {"skills", random_list(skills, 3)},
        {"category", random_list(categories, 2)},
        {"salaryRange", {{"min", min_salary}, {"max", max_salary}}},
        {"location", "Yerevan"},
        {"workType", random_list(workTypes, 1)},
        {"status", "approved"}
    };

    return j;
}

// ------------ MAIN SCRIPT ------------

int main() {
    mongocxx::instance inst{};  // required once per application
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};

    auto db = client["testdb"];
    auto jobsCollection = db["jobs"];

    std::cout << "Connected to MongoDB!" << std::endl;

    int insertedCount = 0;

    for (int i = 0; i < 20; i++) {
        json j = generateRandomJobJson();

        try {
            Job job(j);
            auto bsonDoc = job.toBson();

            auto result = jobsCollection.insert_one(bsonDoc.view());
            if (result) {
                insertedCount++;
                std::cout << "Inserted job #" << insertedCount << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error creating job: " << e.what() << std::endl;
        }
    }

    std::cout << "=====================================" << std::endl;
    std::cout << "Successfully inserted " << insertedCount << " jobs!" << std::endl;
    std::cout << "=====================================" << std::endl;

    return 0;
}
