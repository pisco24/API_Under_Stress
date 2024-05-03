#pragma once

#include <sstream>

#include <json.hpp>
// #include <uuid_v4.h>
// #include <endianness.h>
#include "mongocxx/instance.hpp"
#include "mongodb_handler.hpp"
#include "../Crow/include/crow.h"


class HttpServer {
  public:
    HttpServer() {}

    void setupRoutes() {
      CROW_ROUTE(app, "/")([](){
        return "Hello world";
      });

      CROW_ROUTE(app, "/warrior").methods("POST"_method)
        ([this](const crow::request& req) {
         auto json_body = crow::json::load(req.body);
         if (!json_body) {
          return crow::response(400, "Invalid JSON format");
         }

         std::ostringstream os;                     
         std::string name = json_body["name"].s();                          
         std::string dob = json_body["dob"].s();
         auto fight_skills = const_cast<crow::json::rvalue&> (json_body["fight_skills"]).lo();

         if (name.length() > 100 || dob.length() != 10 || dob[4] != '-' || dob[7] != '-') {
          return crow::response(400, "Bad Request: Invalid name or dob format");
         }

         MongoDbHandler mhandler;
         bool insert_successful = mhandler.AddWarriortoDb(name, dob, fight_skills);

         if (!insert_successful) {
          return crow::response(400, "Failed to insert warrior into database");
         } else {
           crow::response res(201);
           std::string loc("/name/" + id);
           res.add_header("Location", loc);
          return res;
         }
         
      });

      CROW_ROUTE(app, "/warrior/<string>")
      ([](const std::string& id){
        MongoDbHandler mhandler;
        return mhandler.GetDocById(id);
      });

      CROW_ROUTE(app, "/warrior")
      ([](const crow::request& req) {
        const char* term_str = req.url_params.get("t");
        if (!term_str) {
          return crow::response(404, "Bad Request: No search term provided");
        }

        std::string term = std::string(term_str);
        if (term.empty()) {
          return crow::response(404, "Bad Request: Empty search term provided");
        }

        MongoDbHandler mhandler;
        return mhandler.SearchWarriors(term);
    });


      CROW_ROUTE(app, "/counting-warriors")
      ([](){
        MongoDbHandler mhandler;
        const json::JSON &all_documents = mhandler.GetAllDocuments();
        std::ostringstream os;
        os << all_documents;

        return crow::response(os.str());
      });

    }

    void run(int port) {
      app.port(port).multithreaded().run();
    }

  private:
    crow::SimpleApp app;

};
