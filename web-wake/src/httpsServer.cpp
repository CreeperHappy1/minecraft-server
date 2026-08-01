#include "httpsServer.hpp"
#include "idleDaemonIPC.hpp"

httpsServer::httpsServer(std::string listenHost, int listenPort, std::string token, const char* cmd_fifo_path, const char* status_fifo_path)
    : listenHost(listenHost), listenPort(listenPort), token(token), srv("cert.pem", "key.pem"), idIPC(cmd_fifo_path, status_fifo_path, &srv) {
    
    srv.Get("/", [](const auto &, auto &res){res.set_content("ok", "test");});//TEST
    //TODO
}

int httpsServer::run(){
    if(!srv.is_valid()){
        std::cerr << "FATAL: Failed to create https server.\n";
        return 1;
    }

    srv.listen(listenHost, listenPort);
    return 0;
}