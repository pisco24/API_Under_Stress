#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>
#include "constants.hpp"
#include "bsoncxx/builder/stream/document.hpp"
#include "bsoncxx/oid.hpp"
#include "mongocxx/database.hpp"
#include "../Crow/include/crow.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

class MongoDbHandler {
public:
    static MongoDbHandler& getInstance() {
        static MongoDbHandler instance;  // creates only one instance
        return instance;
    }

    // handles "Singleton" method possible errors
    MongoDbHandler(const MongoDbHandler&) = delete;
    MongoDbHandler& operator=(const MongoDbHandler&) = delete;


    crow::response AddWarriortoDb(const std::string &warrior_name, 
                const std::string &warrior_dob, 
                const std::vector<crow::json::rvalue> &warrior_skills) {
      auto builder = bsoncxx::builder::stream::document{};
      auto array_builder = builder << "name" << warrior_name
                                   << "dob" << warrior_dob
                                   << "fight_skills" 
                                   << bsoncxx::builder::stream::open_array;

      int skill_ct = 0;
      for (const auto& skill : warrior_skills) {
        std::string skill_str = skill.s();
        if (skill_str.length() > 250 || skill_ct > 20) {
          return crow::response(400, "Invalid skills.");
        } else {        // add skill check
          array_builder << skill.s();
          skill_ct++;
        }
      }

      bsoncxx::v_noabi::document::value doc_value = 
        array_builder << bsoncxx::builder::stream::close_array << bsoncxx::builder::stream::finalize;

      try {     // execute query
        auto maybe_result = collection.insert_one(doc_value.view());

        if (maybe_result) {
            auto id = maybe_result->inserted_id().get_oid().value.to_string();
            crow::response response = crow::response(201, id);
            response.add_header("Content-Type", "text/plain");
            return response;
        } else {
            return crow::response(500, "Failed to insert document into database.");
        }
      } catch (const std::exception &e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
      }
    }

    crow::response GetDocById(const std::string& id) {
      try {
        bsoncxx::oid oid;
        try {
            oid = bsoncxx::oid{id};  // will throw if the id is not a valid ObjectId
        } catch (const std::exception& e) {
            return crow::response(400, "Invalid ID format"); 
        }

        // create query document
        bsoncxx::builder::basic::document filter{};
        filter.append(bsoncxx::builder::basic::kvp("_id", oid));

        // execute query
        auto maybe_result = collection.find_one(filter.view());

        // create response
        if (maybe_result) {
          std::string result = bsoncxx::to_json(maybe_result->view());
          return crow::response{200, result};
        } else {
          return crow::response(404, "Not found");
        }
      } catch (const std::exception& e) {
        return crow::response(500, std::string("Internal server error: ") + e.what());
      }
    }

    crow::response SearchWarriors(const std::string& term) {
      try {
        auto filter = bsoncxx::builder::basic::make_document(
          bsoncxx::builder::basic::kvp("$text", 
            bsoncxx::builder::basic::make_document(
              bsoncxx::builder::basic::kvp("$search", term)
            )
          )
        );

        // execute query, limit to first 50 results
        mongocxx::options::find opts;
        opts.limit(50);
        auto cursor = collection.find(filter.view(), opts);

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
        return crow::response(500, std::string("Internal server error: ") + e.what());
      }
    }

    crow::response CountDocuments() {
        auto maybe_result = collection.count_documents({});
        if (maybe_result) {
            return crow::response(200, std::to_string(maybe_result));
        } else {
            return crow::response(500, "Failed to count documents.");
        }
    }


    json::JSON GetAllDocuments() {
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
    MongoDbHandler() {
      try {
        mongocxx::instance instance{}; 
        uri = mongocxx::uri(kMongoDbUri);
        client = mongocxx::client(uri);
        db = client[kDatabaseName];
        collection = db[kCollectionName];
        
        _FieldTemplate();
        _CreateTextIndex();
        _CreateUniqueIndex();

      } catch (const mongocxx::exception& e) {
        std::cerr << "MongoDB Setup Initialization Error: " << e.what() << std::endl;
        // fallback logic ?
      }
    }

    void _FieldTemplate() {
      try {
        if (collection.count_documents({}) == 0) {
          auto builder = bsoncxx::builder::stream::document{};
          auto doc_value = builder
            << "name" << "John Doe"
            << "dob" << "1980-01-01"
            << "fight_skills" << bsoncxx::builder::stream::open_array
            << "BJJ" << "KungFu" << "Judo"
            << bsoncxx::builder::stream::close_array
            << bsoncxx::builder::stream::finalize;

          collection.insert_one(doc_value.view());
          std::cout << "Initial document inserted and collection created with fields." << std::endl;
        }
      } catch (const mongocxx::exception& e) {
        std::cerr << "Failed to create collection or insert initial document: " << e.what() << std::endl;
      }
    }

    void _CreateTextIndex() {
      // create a text index on the name, dob and fight_skills fields
      // this is more efficient since we are only using term based searches
      try {
        auto index_builder = bsoncxx::builder::stream::document{};
        bsoncxx::v_noabi::document::value doc_value =
          index_builder << "name" << "text"
                      << "dob" << "text"
                      << "fight_skills" << "text"
                      << bsoncxx::builder::stream::finalize;

        mongocxx::options::index index_options{};
        index_options.background(true); // create the index in the background

        db[kCollectionName].create_index(doc_value.view(), index_options);

        std::cout << "WildcardTextIndex created successfully." << std::endl;
      } catch (const mongocxx::exception& e) {
        std::cerr << "Failed to create WildcardTextIndex: " << e.what() << std::endl;
      }
    }


    void _CreateUniqueIndex() {
      // create a separate unique index on the 'name' field to enforce uniqueness
      try {
        auto builder = bsoncxx::builder::stream::document{};
        bsoncxx::v_noabi::document::value doc_value =
          builder << "name" << 1 << bsoncxx::builder::stream::finalize;
        mongocxx::options::index index_options{};
        index_options.unique(true);
        db[kCollectionName].create_index(doc_value.view(), index_options);

        std::cout << "UniqueIndexForName created successfully." << std::endl;
      } catch (const mongocxx::exception& e) {
        std::cerr << "Failed to create UniqueIndexForName: " << e.what() << std::endl;  
      }
    }


    mongocxx::uri uri;
    mongocxx::client client;
    mongocxx::database db;
    mongocxx::collection collection;
};
