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
    httpsServer(std::string listenHost, int listenPort, std::string token, const char* cmd_fifo_path, const char* status_fifo_path);
    ~httpsServer();

    void run();
};

#endif // HTTP_SERVER_HPP