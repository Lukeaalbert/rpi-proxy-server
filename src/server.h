#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <stddef.h>

#include "cache.h"

class ProxyServer {
    public:

    // Construct ProxyServer with port number (default 12345)
    ProxyServer(size_t port);

    void startServer();

    void runServer();
    
    private:

    // returns http request in string format from client fd.
    // also populates method, uri, and host. 
    std::string parseClientHttpRequest(int clientFd, std::string& method, 
        std::string& uri, std::string& host);
    
    std::string getLastModifiedDate(const std::string& host, const std::string& port,
        const std::string& uri);

    std::string makeGetRequest(const std::string& host, const std::string& path,
        const std::string& port, std::string& responseHeader);
    
    bool writeHttpResponseToClient(int client_fd, const std::string& headers,
        const std::string& data);
    
    std::string readHttpHeader(int socket_fd);

    size_t mPort;
    int mServerFd;

    Cache mCache;
};


#endif //SERVER_H