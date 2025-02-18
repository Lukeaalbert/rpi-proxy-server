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

void ProxyServer::runServer() {

    char clientIp[INET_ADDRSTRLEN];
    std::string clientRequest;
    std::string headerResponse;
    std::string requestedData;
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
            std::cout << "connection established with client at " << clientIp << ".\n\n";
        }

        // get the request from the client, check for errors
        clientRequest = parseClientHttpRequest(clientFd, method, uri, host);
        if (clientRequest.empty() || method.empty() || host.empty() || uri.empty()) {
            std::cerr << "error with request header at " << clientIp << ". skipping...";
            continue;
        }
        if (PRINT_MESSAGES) {
            std::cout << "Request received: \n" << clientRequest;
        }

        // get the cached data if it exists and the last modified date via a head request
        CacheItem* chr = mCache.get(host + uri);
        std::string lastModifiedDate = getLastModifiedDate(host, "80", uri);
        // IF data exists in cache
        if (chr != nullptr) {
            // if data in cache is stale
            if (lastModifiedDate != chr -> lastModified) {
                // get new data, send to client
                requestedData = makeGetRequest(host, "80", uri, headerResponse);
                writeHttpResponseToClient(clientFd, headerResponse, requestedData);
                // edit data in cache to reflect these changes
                chr -> lastModified = std::move(lastModifiedDate);
                chr -> header = std::move(headerResponse);
                chr -> content = std::move(requestedData);
            }
            else { // data in cache exists and is up to date
                writeHttpResponseToClient(clientFd, chr -> header, chr -> content);
            }
        } // Otherwise
        else {
            // get new data, send to client
            requestedData = makeGetRequest(host, "80", uri, headerResponse);
            writeHttpResponseToClient(clientFd, headerResponse, requestedData);
            // edit data in cache to reflect these changes
            mCache.insert(host+uri, headerResponse, requestedData, lastModifiedDate);
        }

        // close client socket and clear stuff
        method.clear();
        host.clear();
        uri.clear();
        headerResponse.clear();
        clientRequest.clear();
        if (PRINT_MESSAGES) {
            std::cout << "Closing client connection" << std::endl;
        }
        shutdown(clientFd, SHUT_RDWR);
        close(clientFd);
    }
        
    close(mServerFd);
}


std::string ProxyServer::parseClientHttpRequest(int clientFd, std::string& method, 
    std::string& uri, std::string& host) {
    std::string line;
    std::string request = std::string();

    // read in the first line of the request header
    int bytes_received = readALine(clientFd, line);
    if (bytes_received == -1) {
        return std::string();
    }

    // get method, full_uri
    std::string full_uri;
    std::istringstream request_stream(line);
    request_stream >> method >> full_uri;

    // parse the full URI to get host and path
    if (full_uri.substr(0, 7) == "http://") {
        size_t host_start = 7;
        size_t host_end = full_uri.find('/', host_start);
        if (host_end != std::string::npos) {
            host = full_uri.substr(host_start, host_end - host_start);
            uri = full_uri.substr(host_end);
        } else {
            host = full_uri.substr(host_start);
            uri = "/";
        }
    }

    request += '\t' + line;

    while (true) {
        bytes_received = readALine(clientFd, line);
        if (bytes_received == -1) {
            return request;
        }
        request += "\t" + line;
        if (bytes_received == 2 && line == "\r\n") {
            return request;
        }
        line.clear();
    }
}

std::string ProxyServer::getLastModifiedDate(const std::string& host, const std::string& port, const std::string& uri) {
    if (PRINT_MESSAGES) {
        std::cout << "Making HEAD request to host: " << host << " uri: " << uri << std::endl << std::endl;
    }

    // 1) open outgoing socket
    int outgoingFd = socket(AF_INET, SOCK_STREAM, 0);
    if (outgoingFd == -1) {
        std::cerr << "Failed to create outgoing socket" << std::endl << std::endl;
        return std::string();
    }

    // 2) create host struct
    struct hostent* host_entry = gethostbyname(host.c_str());
    if (!host_entry) {
        std::cerr << "Failed to resolve host: " << host << std::endl << std::endl;
        close(outgoingFd);
        return std::string();
    }

    // 3) create server address struct
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::stoi(port));
    memcpy(&server_addr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);

    // 4) connect to the outgoing host
    if (connect(outgoingFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to target server" << std::endl << std::endl;
        close(outgoingFd);
        return std::string();
    }

    // 5) build and send request
    std::string request = "HEAD " + uri + " HTTP/1.1\r\n"
                         "Host: " + host + "\r\n"
                         "Connection: close\r\n\r\n";
    if (PRINT_MESSAGES) {
        std::cout << "Sending request:\n" << request;
    }
    send(outgoingFd, request.c_str(), request.length(), 0);

    // 6) read response header
    std::string responseHeader = readHttpHeader(outgoingFd);

    // 7) get last modified date
    std::string lastModDate;
    std::string searchStr = "Last-Modified:";
    auto idx = responseHeader.find(searchStr);
    if (idx != std::string::npos) {
        size_t dateStart = idx + searchStr.length() + 1;
        size_t dateEnd = responseHeader.find("\r\n", dateStart);
        if (dateEnd != std::string::npos) {
            lastModDate = responseHeader.substr(dateStart, dateEnd - dateStart);
        }
    }

    close(outgoingFd);
    return lastModDate;
}


std::string ProxyServer::makeGetRequest(const std::string& host, const std::string& port, 
    const std::string& uri, std::string& responseHeader) {
    if (PRINT_MESSAGES) {
        std::cout << "Making GET request to host: " << host << " uri: " << uri << std::endl << std::endl;
    }

    // 1) open outgoing socket
    int outgoingFd = socket(AF_INET, SOCK_STREAM, 0);
    if (outgoingFd == -1) {
        std::cerr << "Failed to create outgoing socket" << std::endl << std::endl;
        return std::string();
    }

    // 2) create host struct
    struct hostent* host_entry = gethostbyname(host.c_str());
    if (!host_entry) {
        std::cerr << "Failed to resolve host: " << host << std::endl << std::endl;
        close(outgoingFd);
        return std::string();
    }

    // 3) create server address struct
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::stoi(port));
    memcpy(&server_addr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);

    // 4) connect to the outgoing host
    if (connect(outgoingFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to target server" << std::endl << std::endl;
        close(outgoingFd);
        return std::string();
    }

    // 5) build and send request
    std::string request = "GET " + uri + " HTTP/1.1\r\n"
                         "Host: " + host + "\r\n"
                         "Connection: close\r\n\r\n";
    if (PRINT_MESSAGES) {
        std::cout << "Sending request:\n" << request;
    }
    send(outgoingFd, request.c_str(), request.length(), 0);

    // 6) read response header
    responseHeader = readHttpHeader(outgoingFd);

    // 7) read response data
    std::string response;
    int bytes_received;
    char buffer[1024];
    while ((bytes_received = read(outgoingFd, buffer, sizeof(buffer))) > 0) {
        response.append(buffer, bytes_received);
    }

    close(outgoingFd);
    return response;
}

bool ProxyServer::writeHttpResponseToClient(int client_fd, const std::string& headers,
    const std::string& data) {

    size_t total_bytes_written = 0;
    size_t header_length = headers.length();
    size_t data_length = data.length();

    // 1) write the headers
    while (total_bytes_written < header_length) {
        int data_bytes = write(client_fd, headers.c_str() + total_bytes_written, header_length - total_bytes_written);
        if (data_bytes <= 0) {
            // error or connection closed
            return false;
        }
        total_bytes_written += data_bytes;
    }

    if (PRINT_MESSAGES) {
        std::cout << "Sending to client: \n" << headers;
    }

    // 2) write the actual data
    total_bytes_written = 0;

    while (total_bytes_written < data_length) {
        int data_bytes = write(client_fd, data.c_str() + total_bytes_written, data_length - total_bytes_written);
        if (data_bytes <= 0) {
            // error or connection closed
            return false;
        }
        total_bytes_written += data_bytes;
    }
    return true;
}


std::string ProxyServer::readHttpHeader(int outgoingFd) {
    int bytes_received;
    std::string responseHeader = std::string();
    std::string line;
    while ((bytes_received = readALine(outgoingFd, line)) != -1) {
        if (bytes_received == 2 && line == "\r\n") {
            responseHeader += "\r\n";
            break;
        }
        responseHeader += line;
    }
    return responseHeader;
}
