#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <httplib.h>
#include "idleDaemonIPC.hpp"

class httpsServer{
private:
    std::string listenHost;
    int listenPort;
    std::string token;
    idleDaemonIPC idIPC;
public:
    httpsServer(std::string listenHost, int listenPort, std::string token);
    ~httpsServer();

    void run();
};

#endif // HTTP_SERVER_HPP