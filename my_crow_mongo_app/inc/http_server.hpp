#include <sstream>

#include "../Crow/include/crow.h"

class HttpServer {
  public:
    HttpServer() {}

    void setupRoutes() {
      CROW_ROUTE(app, "/warrior").methods("POST"_method)
        ([](const crow::request& req){
         auto json_body = crow::json::load(req.body);
         if (!json_body) return crow::response(400);

         std::ostringstream os;                     
         std::string name = json_body["name"].s();                          
         std::string dob = json_body["dob"].s();                             
         auto fight_skills = const_cast<crow::json::rvalue&> (json_body["fight_skills"]).lo();

         os << "Name: " << name << "\n";
         os << "DOB: " << dob << "\n";
         os << "Fight Skills: ";
         for (const auto& skill : fight_skills) { os << skill << " "; };

         return crow::response(os.str());
      });

      CROW_ROUTE(app, "/warrior/<string>")
        ([](std::string id){
         std::ostringstream os;
         os << "Id: " << id << "\n";
         return crow::response(os.str());
        });

      CROW_ROUTE(app, "/warrior")
      ([](const crow::request& req){
       std::ostringstream os;

       os << "Search term: " << req.url_params << "\n";
       os << "The key 't' was " << (req.url_params.get("t") == nullptr ? "not " : "") << "found.\n";

       return crow::response(os.str());
    });

      CROW_ROUTE(app, "/counting-warriors")
        ([](){
         return "This should simply return the total number of registered warriors in the database.";
      });

    }

    void run(int port) {
      app.port(port).multithreaded().run();
    }

  private:
    crow::SimpleApp app;

};
