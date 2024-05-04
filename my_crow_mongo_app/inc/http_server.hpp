#pragma once

#include <sstream>

#include <json.hpp>
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
            auto x = crow::json::load(req.body);
            if (!x) return crow::response(400, "Invalid JSON format");

            if (!x.has("name") || !x.has("dob") || !x.has("fight_skills"))
                return crow::response(400, "Missing required JSON fields");

            std::ostringstream os;                     
            std::string name = x["name"].s();                          
            std::string dob = x["dob"].s();
            auto fight_skills = const_cast<crow::json::rvalue&> (x["fight_skills"]).lo();

            if (name.length() > 100 || dob.length() != 10 || dob[4] != '-' || dob[7] != '-')
                return crow::response(400, "Bad Request: Invalid name or dob format");

            auto& mhandler = MongoDbHandler::getInstance();
            return mhandler.AddWarriortoDb(name, dob, fight_skills);
        });

        CROW_ROUTE(app, "/warrior/<string>")
        ([](const std::string& id){
            auto& mhandler = MongoDbHandler::getInstance();
            return mhandler.GetDocById(id);
        });

        CROW_ROUTE(app, "/warrior")
        ([](const crow::request& req) {
            const char* term_str = req.url_params.get("t");
            if (!term_str)
                return crow::response(404, "Bad Request: No search term provided");

            std::string term = std::string(term_str);
            if (term.empty()) 
                return crow::response(404, "Bad Request: Empty search term provided");

            auto& mhandler = MongoDbHandler::getInstance();
            return mhandler.SearchWarriors(term);
        });


        CROW_ROUTE(app, "/counting-warriors")
        ([](){
            auto& mhandler = MongoDbHandler::getInstance();
            return mhandler.CountDocuments();
        });

        CROW_ROUTE(app, "/all-warriors")
        ([](){
            auto& mhandler = MongoDbHandler::getInstance();
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
