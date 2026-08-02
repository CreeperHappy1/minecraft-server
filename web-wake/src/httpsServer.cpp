#include "httpsServer.hpp"

bool httpsServer::authenticated(const httplib::Request& req){
    return true;//TODO
}

httpsServer::httpsServer(std::string listenHost, int listenPort, std::string token, const char* cmd_fifo_path, const char* status_fifo_path)
    : listenHost(listenHost), listenPort(listenPort), token(token), 
    srv(certpath.c_str(), keypath.c_str()), idIPC(cmd_fifo_path, status_fifo_path, &srv) {
    
    srv.set_pre_routing_handler([this](const httplib::Request &req, httplib::Response &res){
        for(std::string s : noAuthPatterns)
            if(req.matched_route.rfind(s, 0) == 0)
                return httplib::Server::HandlerResponse::Unhandled;
        if(authenticated(req)){
            return httplib::Server::HandlerResponse::Unhandled;
        }else{
            res.status = 403;
            res.set_content("forbidden", "text/plain");
            return httplib::Server::HandlerResponse::Handled;
        }
    });

    srv.Get("/", [](const httplib::Request &req, httplib::Response &res){
        res.set_content("<script type=\"module\">\n"
                "\timport { bootstrap } from \"/auth.js\";\n"
                "\tbootstrap();"
            "\n</script>",
            "text/html");
    });
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
    srv.Get("/auth.js", [](const httplib::Request &req, httplib::Response &res){
        res.set_file_content(webdir + "auth.js", "application/javascript");
    });
    srv.Get("/script.js", [](const httplib::Request &req, httplib::Response &res){
        res.set_file_content(webdir + "script.js", "application/javascript");
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