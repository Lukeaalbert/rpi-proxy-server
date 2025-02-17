#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>

#include "server.h"

#define MAX_REQUEST_SIZE 1024

// PRINT_MESSAGES = 1: server status messages sent to cout.
// PRINT_MESSAGES = 0: no cout messages.
int PRINT_MESSAGES = 1;

namespace {

    void usage() {
        std::cerr << "Usage: ./ProxyServer port_num\n";
        exit(-1);
    }

    int readALine(int socket_fd, std::string& line) {
        std::string s = std::string();
        int idx = 0;
        char ch = '\0';

        for (;;) {
            int bytes_read = read(socket_fd, &ch, 1);
            if (bytes_read < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return (-1);
            }
            else if (bytes_read == 0) {
                if (idx == 0) {
                    return (-1);
                }
                break;
            } else {
                s += ch;
                idx++;
                if (ch == '\n') {
                    break;
                }
            }
        }
        line = s;
        {
            if (PRINT_MESSAGES) {
                std::cout << "\t" << line;
                std::cout.flush();
            }
        }
        return idx;
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

    if (PRINT_MESSAGES) {
        std::cout << "server successfully started, listening on port " << mPort << "..." << std::endl;
    }
}

std::string ProxyServer::getHttpRequest(int clientFd, std::string& method, 
    std::string& uri, std::string& host)
{
    std::string line;
    std::string request = std::string();
    std::string method;

    // read in the first line of the request header
    int bytes_received = readALine(clientFd, line);
    // returning empty string because this should always work
    if (bytes_received == -1) {
        return std::string();
    }
    // get method, uri, and version.
    std::istringstream request_stream(line);
    request_stream >> method >> uri;

    request += '\t' + line;

    // loop to handle all bytes of single request
    while (true) {
        // read in a line of the request header
        int bytes_received = readALine(clientFd, line);
        // readALine returns -1
        if (bytes_received == -1) {
            return request;
        }
        if (line.substr(0, 5) == "Host:") {
            int idx = 5;
            // strip whitespace
            while (idx < line.size() && line[idx] == ' ') {
                idx++;
            }
            if (idx != line.size() - 1) {
                host = line.substr(idx);
            }
        }
        // build the entire request string (request)
        request += "\t" + line;
       // end of request headers
        if (bytes_received == 2 && line == "\r\n") {
            return request;
        }
        // clear line for next line
        line.clear();
    }

}

std::string ProxyServer::getLastModifiedDate(const std::string& host, const std::string& port, const std::string& uri) {
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return;
    }

    // Get the server address info
    struct hostent* server = gethostbyname(host.c_str());
    if (server == NULL) {
        std::cerr << "No such host " << host << "." << std::endl;
        close(sockfd);
        return;
    }

    // set up the server address structure
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(std::stoi(port));

    // connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        close(sockfd);
        return;
    }

    // prepare the HTTP HEAD request
    std::string request = "HEAD " + uri + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";

    // send the request
    int bytes_sent = send(sockfd, request.c_str(), request.length(), 0);
    if (bytes_sent < 0) {
        std::cerr << "error sending request" << std::endl;
        close(sockfd);
        return;
    }

    // read the response
    std::string line;
    std::string lastModified = std::string();
    int bytes_received;
    while ((bytes_received = readALine(sockfd, line)) != -1) {
        if (line.substr(0, 14) == "Last-Modified:") {
            int idx = 14;
            // strip whitespace
            while (idx < line.size() && line[idx] == ' ') {
                idx++;
            }
            if (idx != line.size() - 1) {
                lastModified = line.substr(idx);
            }
        }
        if (PRINT_MESSAGES) {
            std::cout << line << std::endl;
        }
    }

    if (bytes_received < 0) {
        std::cerr << "Error receiving response" << std::endl;
    }

    // close the socket
    close(sockfd);

    return lastModified;
}

void ProxyServer::runServer() {

    char clientIp[INET_ADDRSTRLEN];
    std::string request;
    std::string method, host, uri;

    while (true) {
        // accept incoming connections
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int clientFd = accept(mServerFd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (clientFd == -1) {
            std::cerr << "accept failed. skipping..." << std::endl;
            continue;
        }

        // get ip address
        inet_ntop(AF_INET, &client_addr.sin_addr, clientIp, INET_ADDRSTRLEN);

        if (PRINT_MESSAGES) {
            std::cout << "connection established with client at " << clientIp << ".\n";
        }

        request = getHttpRequest(clientFd, method, host, uri);

        if (request.empty() || method.empty() || host.empty() || uri.empty()) {
            std::cerr << "error with request header at " << clientIp << ". skipping...";
            continue;
        }
        if (PRINT_MESSAGES) {
            std::cout << "Received request: " << request << std::endl;
        }

        std::string lastModifiedDate = getLastModifiedDate(host, "80", uri);

        // 1) parse requested url
        // 2) check for membership of data in cache
            // IF membership found
                // send HEAD request to url to see if data has been updated
                    // IF data has been updated
                        // get data on behalf of client
                        // send data to client
                        // update data in cache
                    // ELSE
                        // send cached data as a responses
            // ELSE
                // get data on behalf of client
                // send data to client
                // save data in cache
        
        // Do something with request, send repsonse like the following format:

        // const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, Raspberry Pi!";
        // send(clientFd, response, strlen(response), 0);

        method.clear();
        host.clear();
        uri.clear();
        // then, remember to shutdown and close client.
        // shutdown(client_socket_fd, SHUT_RDWR);
        // close(client_socket_fd);
    }
        
    close(mServerFd);
}

