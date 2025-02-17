#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <stddef.h>

class ProxyServer {
    public:

    // Construct ProxyServer with port number (default 12345)
    ProxyServer(size_t port);

    void startServer();

    void runServer();
    
    private:

    // gets http request in string format from client fd.
    // also populates method, uri, and host. 
    std::string getHttpRequest(int clientFd, std::string& method, 
        std::string& uri, std::string& host);
    
    std::string getLastModifiedDate(const std::string& host, const std::string& port,
        const std::string& uri);

    size_t mPort;
    int mServerFd;
};


#endif //SERVER_H