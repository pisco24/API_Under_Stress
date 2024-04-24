#pragma once

#include <cstdint>
#include <string>
#include <iostream>

#include <json.hpp>
#include "bsoncxx/builder/stream/document.hpp"
#include "bsoncxx/json.hpp"
#include "bsoncxx/oid.hpp"
#include "mongocxx/client.hpp"
#include "mongocxx/database.hpp"
#include "mongocxx/uri.hpp"
#include "../Crow/include/crow.h"

constexpr char kMongoDbUri[] = "mongodb://db:27017";
constexpr char kDatabaseName[] = "warrior_db";
constexpr char kCollectionName[] = "WarriorInfo";

class MongoDbHandler{
  public:
    MongoDbHandler()
    : uri(mongocxx::uri(kMongoDbUri)),
      client(mongocxx::client(uri)),
      db(client[kDatabaseName]) {}


    bool addWarriortoDb(const std::string &warrior_name, const std::string &warrior_dob, const std::vector<crow::json::rvalue> &warrior_skills) {
      mongocxx::collection collection = db[kCollectionName];
      auto builder = bsoncxx::builder::stream::document{};

      auto array_builder = builder << "name" << warrior_name
                                   << "dob" << warrior_dob
                                   << "fight_skills" << bsoncxx::builder::stream::open_array;

      int skill_ct = 0;
      for (const auto& skill : warrior_skills) {
        std::string skill_str = skill.s();
        if (skill_str.length() > 250 || skill_ct > 20) {
          return false;
        } else {
          array_builder << skill.s();
          skill_ct++;
        }
      }

      bsoncxx::v_noabi::document::value doc_value = array_builder << bsoncxx::builder::stream::close_array << bsoncxx::builder::stream::finalize;

      try {
        bsoncxx::stdx::optional<mongocxx::result::insert_one> maybe_result = collection.insert_one(doc_value.view());

        if (maybe_result) {
          return maybe_result->inserted_id().get_oid().value.to_string().size() != 0;
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
      if (cursor.begin() != cursor.end()) {
          for (auto doc : cursor) {
          result["warriors"].append(bsoncxx::to_json(doc));
          }
        }
      return result["warriors"];
    }





  private:
    mongocxx::uri uri;
    mongocxx::client client;
    mongocxx::database db;
};
