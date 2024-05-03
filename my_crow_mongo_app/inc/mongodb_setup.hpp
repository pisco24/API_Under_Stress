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

// #include <json.hpp>
#include "bsoncxx/builder/stream/document.hpp"
// #include "bsoncxx/json.hpp"
// #include "bsoncxx/oid.hpp"
// #include "mongocxx/client.hpp"
// #include "mongocxx/database.hpp"
// #include "mongocxx/uri.hpp"
#include "../Crow/include/crow.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;


class MongoDbSetup {
    public:
        MongoDbSetup()
        : uri(mongocxx::uri(kMongoDbUri)),
        client(mongocxx::client(uri)),
        db(client[kDatabaseName]),
        collection(db[kCollectionName]) {
            _indexing();
        }

    private:
        mongocxx::uri uri;
        mongocxx::client client;
        mongocxx::database db;
        mongocxx::collection collection;

        void _indexing() {
            // create a text index on the name, dob and fight_skills fields
            // this is more efficient since we are only using term based searches
            try {
                auto index_builder = bsoncxx::builder::stream::document{};
                index_builder << "name" << "text"
                            << "dob" << "text"
                            << "fight_skills" << "text"
                            << bsoncxx::builder::stream::finalize;

                mongocxx::options::index index_options{};
                index_options.background(true); // create the index in the background

                collection.create_index(index_builder.view(), index_options);

                // create a separate unique index on the 'name' field to enforce uniqueness
                auto unique_index_builder = bsoncxx::builder::stream::document{};
                unique_index_builder << "name" << 1 << bsoncxx::builder::stream::finalize;

                mongocxx::options::index unique_index_options{};
                unique_index_options.unique(true);
                collection.create_index(unique_index_builder.view(), unique_index_options);

                std::cout << "Index created successfully." << std::endl;    // TODO fix logging for docker
            } catch (const mongocxx::exception& e) {
                std::cerr << "Failed to create index: " << e.what() << std::endl;   // TODO fix logging for docker
            }
        }
};
