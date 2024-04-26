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
      db(client[kDatabaseName]) {
        _indexing();
      }

    bool addWarriortoDb(const std::string &warrior_id, const std::string &warrior_name, 
                        const std::string &warrior_dob, const std::vector<crow::json::rvalue> &warrior_skills) {
      mongocxx::collection collection = db[kCollectionName];
      auto builder = bsoncxx::builder::stream::document{};

      auto array_builder = builder << "_id" << 0
                                   << "id" << warrior_id
                                   << "name" << warrior_name
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

      bsoncxx::v_noabi::document::value doc_value = 
        array_builder << bsoncxx::builder::stream::close_array << bsoncxx::builder::stream::finalize;

      try {
        collection.insert_one(doc_value.view());
        return true;
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

    void _indexing() {
      
      // unique index to avoid duplicate entries based off warrior name
      auto builder = bsoncxx::builder::stream::document{};
      bsoncxx::v_noabi::document::value doc_value =
        builder << "name" << 1 << bsoncxx::builder::stream::finalize;
      mongocxx::options::index index_options{};
      index_options.unique(true);
      db[kCollectionName].create_index(doc_value.view(), index_options);
 
    }
};
