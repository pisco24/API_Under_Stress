#include "../inc/http_server.hpp"

int main()
{
  HttpServer server;
  server.setupRoutes();
  server.run(18080);
  return 0;
}
