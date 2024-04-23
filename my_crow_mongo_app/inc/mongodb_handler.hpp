#pragma once

#include <cstdint>
#include <string>
#include <iostream>

#include "SimpleJSON/json.hpp"
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


    bool addWarriortoDb(const std::string &warrior_name, const std::string &warrior_dob, const std::vector<crow::json::rvalue> &warrior_skills) {
      mongocxx::collection collection = db[kCollectionName];
      auto builder = bsoncxx::builder::stream::document document{};

      bsoncxx::v_noabi::document::value doc_value =
        builder << "name" << warrior_name
                << "dob" << warrior_dob
                << "fight_skills" << bsoncxx::builder::stream::open_array;

      for (const auto& skill : warrior_skills) {
        builder << skill.s();
      }

      builder << bsoncxx::builder::stream::close_array << bsoncxx::builder::stream::finalize;

      try {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> maybe_result = collection.insert_one(doc_value.view());

        if (maybe_result) {
          return maybe_result->inserted_id().get_oid().value.to_string().size != 0;
        }
      } catch (const std::exception &e) {
        return false;
      }
    }

    json::JSON GetAllDocuments() {
      mongocxx::collection collection = db[kCollectionName];
      mongocxx::cursor cursor = collection.find({});
      json::JSON result;
      result["warriors"] = json::Array();
      if (cursor.begin() != cursor.end() {
          for (auto doc : cursor) {
          result["warriors"].append(bsoncxx::to_json(doc));
          }
        }
      return result;
    }





  private:
    mongocxx::uri uri;
    mongocxx::client client;
    mongocxx::database db;
};
