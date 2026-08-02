#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include "idleDaemonIPC.hpp"

#ifdef WEB_DIR
const std::string webdir = WEB_DIR;
#else
const std::string webdir = "";//fallback
#endif
const std::string certpath = webdir+"cert.pem";
const std::string keypath = webdir+"key.pem";

class httpsServer{
private:
    httplib::SSLServer srv;
    std::string listenHost;
    int listenPort;
    std::string token;
    idleDaemonIPC idIPC;

    bool static authenticated(const httplib::Request& req);
public:
    httpsServer(std::string listenHost, int listenPort, std::string token, const char* cmd_fifo_path, const char* status_fifo_path);
    ~httpsServer();

    int run();
};

#endif // HTTP_SERVER_HPP