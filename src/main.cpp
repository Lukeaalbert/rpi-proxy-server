#include<string>
#include "server.h"

int main(int argc, char* argv[]) {
   int port = std::stoi(argv[1]);
   ProxyServer ps(port);
   ps.startServer();
   ps.runServer();
   return 0;
}