#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include "idleDaemonIPC.hpp"

class httpsServer{
private:
    httplib::SSLServer srv;
    std::string listenHost;
    int listenPort;
    std::string token;
    idleDaemonIPC idIPC;
    
public:
    httpsServer(std::string listenHost, int listenPort, std::string token, const char* cmd_fifo_path, const char* status_fifo_path);
    ~httpsServer();

    int run();
};

#endif // HTTP_SERVER_HPP