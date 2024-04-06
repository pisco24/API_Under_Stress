// main.cc 
#include <lithium_http_server.hh>
#include <lithium_json.hh>
#include <symbols.hh>
#include <string>

LI_SYMBOL(date_of_birth)
LI_SYMBOL(fight_skills)

int main() {
  li::http_api my_api;

  /* POST /warrior - creates warrior;
   * name         mandatory, (string) max 100 chars
   * DOB          mandatory, (string) date in YYYY-DD-MM format
   * fight_skills mandatory, array with max 20 entries of strings max 250 chars
   *
   * API must return status code 201 - created with header "Location: /name/[:id]" where [:id] is the id
   * in UUID format of any version - from the client just created.  Body and return is at developer discretion.
   */
  my_api.post("/warrior") = [&](li::http_request& request, li::http_response& response) {
    auto params = request.post_parameters(s::name = std::string(), s::date_of_birth = std::string(), s::fight_skills = std::string());
    response.write("hello " + params.name + params.date_of_birth + params.fight_skills);
  };

  /* GET /warrior/[:id] - return warrior created with corresponding id;
   * Return 200 - OK or 404 - Not Found
   */

  /* GET /warrior?t=[:search term] - search warrior attributes
   * Always return 200, even with an empty list.  Results don't need to be paginated. It should
   * return the first 50 entries. If no seach term passed, the route must return 400 - Bad Request.
   */


  /* GET /counting-warriors - count registered warriors;
   * Route just to verify proper test operations, doesn't need to have any performance concern, and it'll
   * be not the subject of evaluation
   */
  my_api.get("/counting-warriors") = [&](li::http_request& request, li::http_response& response) {
    response.write("Implement this later.");
  };
  li::http_serve(my_api, 8080);
}

