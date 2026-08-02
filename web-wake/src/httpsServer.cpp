#include "httpsServer.hpp"
#include "idleDaemonIPC.hpp"

#ifdef CONFIG_PATH
const std::string webdir = WEBDIR;
#else
const std::string webdir = "";//fallback
#endif
const std::string certpath = webdir+"cert.pem";
const std::string keypath = webdir+"key.pem";

httpsServer::httpsServer(std::string listenHost, int listenPort, std::string token, const char* cmd_fifo_path, const char* status_fifo_path)
    : listenHost(listenHost), listenPort(listenPort), token(token), srv(certpath.c_str(), keypath.c_str()), idIPC(cmd_fifo_path, status_fifo_path, &srv) {
    
    srv.Get("/", [](const auto &, auto &res){res.set_content("ok", "test");});//TEST
    //TODO
}

httpsServer::~httpsServer(){ }

int httpsServer::run(){
    if(!srv.is_valid()){
        std::cerr << "FATAL: Failed to create https server.\n";
        return 1;
    }

    srv.listen(listenHost, listenPort);
    return 0;
}