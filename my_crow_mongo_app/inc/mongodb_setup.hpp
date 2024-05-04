#pragma once

// #include <cstdint>
// #include <iostream>
// #include <vector>

#include <bsoncxx/builder/basic/document.hpp>
#include "bsoncxx/builder/stream/document.hpp"
// #include <bsoncxx/json.hpp>
// #include <mongocxx/client.hpp>
// #include <mongocxx/instance.hpp>
// #include <mongocxx/stdx.hpp>
// #include <mongocxx/uri.hpp>
// #include <mongocxx/exception/exception.hpp>
// #include "bsoncxx/oid.hpp"
// #include "mongocxx/database.hpp"
// #include "constants.hpp"
// #include "../Crow/include/crow.h"

#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/stdx.hpp>
#include "constants.hpp"

class MongoDbSetup {
public:
    MongoDbSetup() {
        try {
            uri = mongocxx::uri(kMongoDbUri);
            client = mongocxx::client(uri);
            db = client[kDatabaseName];

            if (!client) {
                throw std::runtime_error("Failed to establish a MongoDB client connection.");
            }
        } catch (const std::exception& e) {
            std::cerr << "MongoDB Setup Initialization Error: " << e.what() << std::endl;
        }

        try {
            // create the collection if it does not exist
            auto collection = db.create_collection(kCollectionName);
            auto builder = bsoncxx::builder::stream::document{};
            bsoncxx::document::value doc_value = builder
                << "name" << "John Doe"
                << "dob" << "1980-01-01"
                << "fight_skills" << bsoncxx::builder::stream::open_array
                << "BJJ" << "KungFu" << "Judo"
                << bsoncxx::builder::stream::close_array
                << bsoncxx::builder::stream::finalize;
                
            collection.insert_one(doc_value.view());

            std::cout << "Document inserted and collection created with fields." << std::endl;
                
            _indexing(collection);
        } catch (const mongocxx::exception& e) {
            std::cerr << "Failed to create collection: " << e.what() << std::endl;
        }


        _indexing(db.collection(kCollectionName));
    }

private:
    void _indexing(mongocxx::collection& collection) {
        try {
            // Create a text index on the name, dob and fight_skills fields
            bsoncxx::builder::stream::document index_builder;
            index_builder << "name" << "text"
                          << "dob" << "text"
                          << "fight_skills" << "text"
                          << bsoncxx::builder::stream::finalize;

            mongocxx::options::index index_options;
            index_options.background(true);
            collection.create_index(index_builder.view(), index_options);

            std::cout << "Indexes created successfully." << std::endl;
        } catch (const mongocxx::exception& e) {
            std::cerr << "Failed to create index: " << e.what() << std::endl;
        }
    }
};


// class MongoDbSetup {
// public:
//     MongoDbSetup() {
//         try {
//             uri = mongocxx::uri(kMongoDbUri);
//             client = mongocxx::client(uri);
//             db = client[kDatabaseName];
//             collection = db[kCollectionName];

//             if (!client) {
//                 throw std::runtime_error("Failed to establish a MongoDB client connection.");
//             }

//             _indexing();
//         } catch (const std::exception& e) {
//             std::cerr << "MongoDB Setup Initialization Error: " << e.what() << std::endl;
//             // Consider rethrowing or handling to prevent further execution.
//         }
//     }

//     private:
//         mongocxx::uri uri;
//         mongocxx::client client;
//         mongocxx::database db;
//         mongocxx::collection collection;

//         void _indexing() {
//             // create a text index on the name, dob and fight_skills fields
//             // this is more efficient since we are only using term based searches
//             try {
//                 auto index_builder = bsoncxx::builder::stream::document{};
//                 index_builder << "name" << "text"
//                             << "dob" << "text"
//                             << "fight_skills" << "text"
//                             << bsoncxx::builder::stream::finalize;

//                 mongocxx::options::index index_options{};
//                 index_options.background(true); // create the index in the background

//                 collection.create_index(index_builder.view(), index_options);

//                 // create a separate unique index on the 'name' field to enforce uniqueness
//                 // auto unique_index_builder = bsoncxx::builder::stream::document{};
//                 // unique_index_builder << "name" << 1 << bsoncxx::builder::stream::finalize;

//                 // mongocxx::options::index unique_index_options{};
//                 // unique_index_options.unique(true);
//                 // collection.create_index(unique_index_builder.view(), unique_index_options);

//                 std::cout << "Index created successfully." << std::endl;    // TODO fix logging for docker
//             } catch (const mongocxx::exception& e) {
//                 std::cerr << "Failed to create index: " << e.what() << std::endl;   // TODO fix logging for docker
//             }
//         }
// };
