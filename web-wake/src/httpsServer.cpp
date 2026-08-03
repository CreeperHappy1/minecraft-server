#include "httpsServer.hpp"

bool httpsServer::authenticated(const httplib::Request& req){
    std::string attemptToken = req.get_header_value("Authorization");
    if(attemptToken.rfind("Bearer ", 0) != 0)
        return false;//it doesn't start with "Bearer "
    attemptToken.erase(0, 7);
    return (attemptToken == token);
}

httpsServer::httpsServer(std::string listenHost, int listenPort, std::string token, const char* cmd_fifo_path, const char* status_fifo_path)
    : listenHost(listenHost), listenPort(listenPort), token(token), 
    srv(certpath.c_str(), keypath.c_str()), idIPC(cmd_fifo_path, status_fifo_path, &srv) {
    
    srv.set_pre_request_handler([this](const httplib::Request &req, httplib::Response &res){
        for(std::string s : noAuthPatterns)
            if(req.path == s)
                return httplib::Server::HandlerResponse::Unhandled;
        if(authenticated(req)){
            return httplib::Server::HandlerResponse::Unhandled;
        }else{
            res.status = 403;
            res.set_content("forbidden", "text/plain");
            return httplib::Server::HandlerResponse::Handled;
        }
    });

    std::ifstream html(webdir + "panel.html");
    if(!html.is_open()){
        std::cerr << "ERROR: Failed to open " << webdir << "panel.html\n";
        parsedHtml = "";
    }else{
        std::stringstream buffer;
        buffer << html.rdbuf();
        html.close();
        parsedHtml = buffer.str();

        std::ifstream script(webdir + "script.js");
        if(!script.is_open()){
            std::cerr << "ERROR: Failed to open " << webdir << "script.js\n";
        }else{
            int pos = parsedHtml.find(scriptPlaceholder);
            buffer.str(std::string());//clear the buffer
            buffer << script.rdbuf();
            script.close();
            if(pos == std::string::npos){
                std::cerr << "ERROR: Didn't find the substitution marker \"" << scriptPlaceholder << "\" in panel.html\n";
            }else{
                parsedHtml = parsedHtml.substr(0, pos) + buffer.str() + parsedHtml.substr(pos+scriptPlaceholder.length());
            }
        }
    }

    srv.Get("/", [](const httplib::Request &req, httplib::Response &res){
        res.set_content("<script type=\"module\">\n"
                "\timport { bootstrap } from \"/auth.js\";\n"
                "\tbootstrap();"
            "\n</script>",
            "text/html");
    });
    srv.Get("/panel.html", [this](const httplib::Request &req, httplib::Response &res){
        res.set_content(parsedHtml, "text/html");
    });
    srv.Get("/status", [this](const httplib::Request &req, httplib::Response &res){
        res.set_content(idIPC.getFullStatus().dump(),"application/json");
    });
    srv.Post("/wake", [this](const httplib::Request &req, httplib::Response &res){
        idIPC.wake();
    });
    srv.Get("/setup", [](const httplib::Request &req, httplib::Response &res){
        res.set_content("<script src=\"setup.js\">\n"
            "\n</script>",
            "text/html");
    });
    srv.Get("/setup.js", [](const httplib::Request &req, httplib::Response &res){
        res.set_file_content(webdir + "setup.js", "application/javascript");
    });
    srv.Get("/auth.js", [](const httplib::Request &req, httplib::Response &res){
        res.set_file_content(webdir + "auth.js", "application/javascript");
    });
    srv.Get("/script.js", [](const httplib::Request &req, httplib::Response &res){
        res.set_file_content(webdir + "script.js", "application/javascript");
    });
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