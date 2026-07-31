#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "../include/httplib.h"
#include "idleDaemonIPC.hpp"

class httpsServer{
private:
    std::string listen;
    int port;
    std::string token;
    idleDaemonIPC idIPC;
public:
    httpsServer(std::string listen, int port, std::string token);
    ~httpsServer();
};

#endif // HTTP_SERVER_HPP