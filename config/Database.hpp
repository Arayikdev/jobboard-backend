#pragma once

#include "../utils/Env.hpp"
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <iostream>
#include <string>

class Database {
private:
    static mongocxx::instance inst;   // One instance globally
    mongocxx::client client;
    mongocxx::database db;

public:
    Database() {
        // Берём переменные окружения
        std::string uri_str = Env::get("DB_URI");
        std::string db_name = Env::get("DB_NAME", "jobboard");

        if (uri_str.empty()) {
            throw std::runtime_error("ERROR: DB_URI is not set!");
        }

        std::cout << "[DB] Connecting to MongoDB Atlas...\n";
        std::cout << "[DB] URI: " << uri_str << std::endl;

        mongocxx::uri uri{uri_str};
        client = mongocxx::client{uri};
        db = client[db_name];

        std::cout << "[DB] Connected to database: " << db_name << std::endl;
    }

    mongocxx::database& getDb() { return db; }
    mongocxx::collection getCollection(const std::string& name) { return db[name]; }
};

inline mongocxx::instance Database::inst{};
