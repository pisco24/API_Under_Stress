#include "../Crow/include/crow.h"
#include <sstream>

int main()
{
    crow::SimpleApp app;


  /* POST /warrior - creates warrior;
   * name         mandatory, (string) max 100 chars
   * DOB          mandatory, (string) date in YYYY-DD-MM format
   * fight_skills mandatory, array with max 20 entries of strings max 250 chars
   *
   * API must return status code 201 - created with header "Location: /name/[:id]" where [:id] is the id
   * in UUID format of any version - from the client just created.  Body and return is at developer discretion.
   */

    CROW_ROUTE(app, "/warrior").methods("POST"_method)
      ([](const crow::request& req){
       auto json_body = crow::json::load(req.body);
       if (!json_body) return crow::response(400);
       
       std::ostringstream os;
       std::string name = json_body["name"].s();
       std::string dob = json_body["dob"].s();
       auto fight_skills = const_cast<crow::json::rvalue&>(json_body["fight_skills"]).lo(); // this should be a list, change later
       os << "Name: " << name << "\n";
       os << "DOB: " << dob << "\n";
       os << "Fight Skills: ";
       for (const auto& element : fight_skills) { os << element << " "; };

       return crow::response(os.str());
       });


  /* GET /warrior/[:id] - return warrior created with corresponding id;
   * Return 200 - OK or 404 - Not Found
   */

    //CROW_ROUTE(app, "/warrior/<>")
      //([](const std::string& id){
       //std::ostringstream os;
       //os << "Id: " << id << "\n";
      //return crow::response(os.str());
    //});


  /* GET /warrior?t=[:search term] - search warrior attributes
   * Always return 200, even with an empty list.  Results don't need to be paginated. It should
   * return the first 50 entries. If no seach term passed, the route must return 400 - Bad Request.
   */

    CROW_ROUTE(app, "/warrior")
      ([](const crow::request& req){
       std::ostringstream os;

        // To get a simple string from the url params
        // To see it in action /warrior?t='search-attributes'
        os << "You searched for " << req.url_params << "\n\n";
        os << "The key 't' was " << (req.url_params.get("t") == nullptr ? "not " : "") << "found.\n";

        // response should include search results
        return crow::response(os.str());
    });


  /* GET /counting-warriors - count registered warriors;
   * Route just to verify proper test operations, doesn't need to have any performance concern, and it'll
   * be not the subject of evaluation
   */

    CROW_ROUTE(app, "/counting-warriors")([](){
        return "This should simply return the total number of registered warriors in the database.";
    });

    app.port(18080).multithreaded().run();
}
