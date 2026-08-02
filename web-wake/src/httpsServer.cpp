#include "httpsServer.hpp"

bool httpsServer::authenticated(const httplib::Request& req){
    return true;//TODO
}

httpsServer::httpsServer(std::string listenHost, int listenPort, std::string token, const char* cmd_fifo_path, const char* status_fifo_path)
    : listenHost(listenHost), listenPort(listenPort), token(token), 
    srv(certpath.c_str(), keypath.c_str()), idIPC(cmd_fifo_path, status_fifo_path, &srv) {
    
    srv.set_pre_routing_handler([this](const httplib::Request &req, httplib::Response &res){
        if(req.matched_route.rfind("/", 0) != 0 && req.matched_route.rfind("/setup", 0) != 0){//only / and /setup are allowed without auth
            if(!authenticated(req)){
                res.status = 403;
                res.set_content("forbidden", "text/plain");
                return httplib::Server::HandlerResponse::Handled;
            }
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });
    srv.Get("/", [](const auto &, auto &res){res.set_content("ok", "test");});//TEST//TODO
    srv.Get("/panel.html", [](const httplib::Request &req, httplib::Response &res){
        res.set_file_content(webdir + "panel.html", "text/html");
    });
    srv.Get("/status", [this](const httplib::Request &req, httplib::Response &res){
        res.set_content(idIPC.getFullStatus().dump(),"application/json");
    });
    srv.Post("/wake", [this](const httplib::Request &req, httplib::Response &res){
        idIPC.wake();
    });
    srv.Get("/setup", [](const httplib::Request &req, httplib::Response &res){
        res.set_file_content(webdir + "setup.js", "application/javascript");
    });
    //TODO
}

httpsServer::~httpsServer(){ }

int httpsServer::run(){
    if(!srv.is_valid()){
        std::cerr << "FATAL: Failed to create https server.\n";
        ERR_print_errors_fp(stderr);
        return 1;
    }

    srv.listen(listenHost, listenPort);
    return 0;
}