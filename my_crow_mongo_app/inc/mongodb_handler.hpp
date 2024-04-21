#pragma once

#include <cstdint>
#include <string>
#include <iostream>

#include "bsoncxx/builder/stream/document.hpp"
#include "bsoncxx/json.hpp"
#include "bsoncxx/oid.hpp"
#include "mongocxx/client.hpp"
#include "mongocxx/database.hpp"
#include "mongocxx/uri.hpp"

constexpr char kMongoDbUri[] = "mongodb://0.0.0.0:27017";
constexpr char kDatabaseName[] = "warrior_db";
constexpr char kCollectionName[] = "WarriorInfo";

class MongoDbHandler{
  public:
    : uri(mongocxx::uri(kMongoDbUri)),
      client(mongocxx::client(uri)),
      db(client[kDatabaseName]) {}


    bool addWarriortoDb(const std::string &warrior_name, const std::string &warrior_dob, 






  private:
    mongocxx::uri uri;
    mongocxx::client client;
    mongocxx::database db;
};
