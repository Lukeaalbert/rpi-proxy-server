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
    std::string getHttpRequest(int clientFd);

    size_t mPort;
    int mServerFd;
};


#endif //SERVER_H