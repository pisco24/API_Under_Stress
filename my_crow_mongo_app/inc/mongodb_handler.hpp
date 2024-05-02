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

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

class MongoDbHandler{
  public:

    MongoDbHandler()
    : uri(mongocxx::uri(kMongoDbUri)),
      client(mongocxx::client(uri)),
      db(client[kDatabaseName]) {
        _indexing();
      }

    bool AddWarriortoDb(const std::string &warrior_id, const std::string &warrior_name, 
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

    crow::response GetDocById(const std::string& id) {
      try {
        mongocxx::collection coll = db[kCollectionName];

        // create the query document
        bsoncxx::builder::basic::document filter{};
        filter.append(bsoncxx::builder::basic::kvp("id", id));

        // execute query
        auto maybe_result = coll.find_one(filter.view());
        if (maybe_result) {
          std::string result = bsoncxx::to_json(maybe_result->view());
          crow::response response{result};
          response.add_header("Content-Type", "application/json");
          return crow::response{result};
        } else {
          return crow::response(404, "Not found");
        }
      } catch (const std::exception& e) {
        return crow::response(500, std::string("Internal server error: ") + e.what());
      }
    }

    crow::response SearchWarriors(const std::string& term) {
      mongocxx::collection coll = db[kCollectionName];
      try {
        auto filter = bsoncxx::builder::basic::make_document(
          bsoncxx::builder::basic::kvp("name", 
            bsoncxx::builder::basic::make_document(
              bsoncxx::builder::basic::kvp("$regex", term),
              bsoncxx::builder::basic::kvp("$options", "i")
            )
          )
        );

        // execute query, limit to first 50 results
        mongocxx::options::find opts;
        opts.limit(50);
        auto cursor = coll.find(filter.view(), opts);

        // prepare JSON response
        crow::json::wvalue results = crow::json::wvalue::list({});
        size_t index = 0;
        for (const auto& doc : cursor) {
          results[index++] = crow::json::load(bsoncxx::to_json(doc));
        }

        if (index == 0) {
          return crow::response(200, "[]"); // return empty array if no results
        }

        return crow::response{results};
      } catch (const std::exception& e) {
        // Handle exceptions from MongoDB or document conversion
        return crow::response(500, std::string("Internal server error: ") + e.what());
      }
    }


    json::JSON GetAllDocuments() {
      mongocxx::collection coll = db[kCollectionName];
      mongocxx::cursor cursor = coll.find({});
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
