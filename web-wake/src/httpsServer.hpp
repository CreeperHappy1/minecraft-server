#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "../include/httplib.h"
#include "idleDaemonSocket.hpp"

class httpsServer{
private:
    std::string listen;
    int port;
    std::string token;
    idleDaemonSocket idleSoc;
public:
    httpsServer(std::string listen, int port, std::string token);
    ~httpsServer();
};

#endif // HTTP_SERVER_HPP