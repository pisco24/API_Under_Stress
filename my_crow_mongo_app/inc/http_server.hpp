#pragma once

#include <sstream>

#include <json.hpp>
#include <uuid_v4.h>
#include <endianness.h>
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
          return crow::response(400);
         }

         std::ostringstream os;                     
         std::string name = json_body["name"].s();                          
         std::string dob = json_body["dob"].s();
         std::string id = generateUUID(); 
         auto fight_skills = const_cast<crow::json::rvalue&> (json_body["fight_skills"]).lo();

         if (name.length() > 100 || dob.length() != 10 || dob[4] != '-' || dob[7] != '-') {
          return crow::response(400);
         }

         MongoDbHandler mhandler;
         bool insert_successful = mhandler.addWarriortoDb(id, name, dob, fight_skills);

         if (!insert_successful) {
          return crow::response(400);
         } else {
           crow::response res(201);
           std::string loc("/name/" + id);
           res.add_header("Location", loc);
          return res;
         }
         
      });

      CROW_ROUTE(app, "/warrior/<string>")
        ([](std::string id){
         std::ostringstream os;
         os << "Id: " << id << "\n";

         crow::json::wvalue x ({
             {"id", id},
             {"name", "Master Yoda"},
             {"dob", "1900-12-12"},
             {"fight_skills", "blah blah"}
          });
         return x;
        });

      CROW_ROUTE(app, "/warrior")
      ([](const crow::request& req){
       std::ostringstream os;

       os << "Search term: " << req.url_params << "\n";
       os << "The key 't' was " << (req.url_params.get("t") == nullptr ? "not " : "") << "found.\n";

       crow::json::wvalue a({
           {"name", "Bob"},
           {"dob", "1999-12-12"},
           {"fight_skills", "blah"}
        });

       crow::json::wvalue b({
           {"name", "Sam"},
           {"dob", "2000-01-01"},
           {"fight_skills", "blahblah"}
        });

       crow::json::wvalue my_list(crow::json::wvalue::list({a, b}));
       return my_list;
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

    UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;

    std::string generateUUID() {
      UUIDv4::UUID uuid = uuidGenerator.getUUID();
      std::string s = uuid.str();
      return s;
    }

};
