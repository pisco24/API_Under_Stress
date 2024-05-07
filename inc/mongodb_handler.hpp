#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>
#include "bsoncxx/builder/stream/document.hpp"
#include "constants.hpp"
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
        int total_skill_length = 0;
        for (const auto& skill : warrior_skills) {
            if (!isValidSkill(skill.s())) {
                return crow::response(400, "Invalid skills.");
            }
            total_skill_length += skill.s().length();
            if (total_skill_length > 250 || warrior_skills.size() > 20) {
                return crow::response(400, "Skill data exceeds limits.");
            }
        }

        bsoncxx::builder::stream::document builder{};
        bsoncxx::builder::stream::array array_builder{};

        for (const auto& skill : warrior_skills) {
            array_builder << skill.s();
        }

        bsoncxx::v_noabi::document::value doc_value = builder
            << "name" << warrior_name
            << "dob" << warrior_dob
            << "fight_skills" << array_builder
            << bsoncxx::builder::stream::finalize;

        try {
            auto maybe_result = collection.insert_one(doc_value.view());
            if (maybe_result) {
                auto id = maybe_result->inserted_id().get_oid().value.to_string();
                crow::response response = crow::response(201);
                response.add_header("location", ":8080/warrior/" + id);
                return response;
            } else {
                return crow::response(400, "Failed to insert doc into db.");
            }
        } catch (const std::exception &e) {
            return crow::response(500, std::string("Internal server error: ") + e.what());
        }
    }

 
    crow::response GetDocById(const std::string& id) {
        try {
            bsoncxx::oid oid;
            try {
                oid = bsoncxx::oid{id};  // throws if id is not valid ObjectId
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
            return crow::response(404, std::string("Internal server error: ") + e.what());
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
        return crow::response(400, std::string("Internal server error: ") + e.what());
      }
    }

    crow::response CountDocuments() {
        auto maybe_result = collection.count_documents({});
        if (maybe_result) {
            return crow::response(200, std::to_string(maybe_result));
        } else {
            return crow::response(400, "Failed to count documents.");
        }
    }

    bool isValidSkill(const std::string& skill) {
        return valid_skills.find(skill) != valid_skills.end();
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

        std::cout << "TextIndex created successfully." << std::endl;
      } catch (const mongocxx::exception& e) {
        std::cerr << "Failed to create TextIndex: " << e.what() << std::endl;
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


   // crow::response AddWarriortoDb(const std::string &warrior_name, 
    //             const std::string &warrior_dob, 
    //             const std::vector<crow::json::rvalue> &warrior_skills) {
    //     auto builder = bsoncxx::builder::stream::document{};
    //     auto array_builder = builder << "name" << warrior_name
    //                             << "dob" << warrior_dob
    //                             << "fight_skills" 
    //                             << bsoncxx::builder::stream::open_array;

    //     int skill_ct = 0;
    //     int skill_len = 0;
    //     for (const auto& skill : warrior_skills) {
    //         if (!isValidSkill(skill.s()) || skill_len > 250 || skill_ct > 20) {
    //             return crow::response(400, "Invalid skills.");
    //         } else {
    //             array_builder << skill.s();
    //             skill_ct++;
    //             std::string skill_str = skill.s();
    //             skill_len += skill_str.length();
    //         }
    //     }

    //     bsoncxx::v_noabi::document::value doc_value = array_builder 
    //         << bsoncxx::builder::stream::close_array 
    //         << bsoncxx::builder::stream::finalize;

    //     try {
    //         auto maybe_result = collection.insert_one(doc_value.view());

    //         if (maybe_result) {
    //             auto id = maybe_result->inserted_id().get_oid().value.to_string();
    //             crow::response response = crow::response(201);
    //             std::string loc(":8080/warrior/" + id);
    //             response.add_header("location", loc);
    //             return response;
    //         } else {
    //             return crow::response(400, "Failed to insert doc into db.");
    //         }
    //     } catch (const std::exception &e) {
    //         std::string error = std::string("Internal server error: ") + e.what();
    //         return crow::response(400, error);
    //     }
    // }
