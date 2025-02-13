#include <iostream>
#include <fstream>
#include <string>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

#include "server.h"

#define MAX_REQUEST_SIZE 1024

namespace {
    void usage() {
        std::cerr << "Usage: ./ProxyServer port_num\n";
        exit(-1);
    }

    long getAvailableRamMemory() {
        std::ifstream meminfo("/proc/meminfo");
        std::string line;
        long availableMemory = 0;

        while (std::getline(meminfo, line)) {
            if (line.find("MemAvailable") != std::string::npos) {
                sscanf(line.c_str(), "MemAvailable: %ld kB", &availableMemory);
                return availableMemory;
            }
        }

        return -1;
    }
}

ProxyServer::ProxyServer(size_t port): mPort(port) {}

void ProxyServer::startServer() {

    // 1) create socket
    mServerFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mServerFd == -1) {
        std::cerr << "socket creation failed. exiting..." << std::endl;
        exit(-1);
    }

    // pretty boiler plate stuff to setup server address struct
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET; // <- IPV4
    server_addr.sin_addr.s_addr = INADDR_ANY; // <- listen on all interfaces
    server_addr.sin_port = htons(mPort);  // <- set port

    // 2) bind socket to address
    if (bind(mServerFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "binding failed. exiting..." << std::endl;
        close(mServerFd);
        exit(-1);
    }

    // 3) listen for connections (max 5 queued connections)
    if (listen(mServerFd, 5) == -1) {
        std::cerr << "listen failed. exiting..." << std::endl;
        close(mServerFd);
        exit(-1);
    }


    std::cout << "server successfully started, listening on port " << mPort << "..." << std::endl;
}

void ProxyServer::runServer() {

    char clientIp[INET_ADDRSTRLEN];
    std::string request;

    while (true) {
        // accept incoming connections
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int clientFd = accept(mServerFd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (clientFd == -1) {
            std::cerr << "accept failed. continuing..." << std::endl;
            continue;
        }

        // get ip address
        inet_ntop(AF_INET, &client_addr.sin_addr, clientIp, INET_ADDRSTRLEN);
        std::cout << "connection established with client at " << clientIp << ".\n";

        request = getHttpRequest(clientFd);
        if (!request.empty()) {
            // DELETE ME! just here for testing.
            std::cout << request << std::endl;
        }

        // 1) parse requested url
        // 2) send HEAD request to url to see if data has been updated
        // 3) check for membership of data in cache
            // IF membership found AND no updates to data
                // send cached data as a responses
            // ELSE
                // get data on behalf of client
                // send data to client
                // save data in cache
        
        // Do something with request, send repsonse like the following format:

        // const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, Raspberry Pi!";
        // send(clientFd, response, strlen(response), 0);

        // then, remember to close client.
        // close(clientFd);
    }
        
    close(mServerFd);
}

std::string ProxyServer::getHttpRequest(int clientFd) {
    char buffer[MAX_REQUEST_SIZE];
    
    // clear buffer
    memset(buffer, 0, MAX_REQUEST_SIZE);

    // read the incoming request
    ssize_t bytes_received = recv(clientFd, buffer, MAX_REQUEST_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("error reading request.");
        close(clientFd);
        return std::string();
    }

    return std::string(buffer);
}