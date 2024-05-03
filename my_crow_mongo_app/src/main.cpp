#include "../inc/http_server.hpp"
#include "../inc/mongodb_setup.hpp"

int main()
{
  MongoDbSetup db_setup;
  HttpServer server;
  server.setupRoutes();
  server.run(18080);
  return 0;
}
