#include<string>
#include <iostream>
#include <stdexcept>
#include "server.h"

void usage() {
   std::cerr << "Usage: ./proxy-server [SERVER PORT NUMBER]\n";
}

int main(int argc, char* argv[]) {
   if (argc != 2) {
      usage();
   }
   else {
      try {
         int port = std::stoi(argv[1]);
         ProxyServer ps(port);
         ps.startServer();
         ps.runServer();
      }
      catch (const std::invalid_argument& e) {
         usage();
      }
   }
   return 0;
}