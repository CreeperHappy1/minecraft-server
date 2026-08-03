#include "idleDaemonIPC.hpp"
#include <chrono>

idleDaemonIPC::idleDaemonIPC(const char* cmd_fifo_path, const char* status_fifo_path, httplib::Server* server){
    write_fd.open(cmd_fifo_path);
    read_fd.open(status_fifo_path);
    this->server = server;
    if(!write_fd.is_open()){
        std::cerr << "FATAL: Failed to open fifo to send wake to the server.\n";
        server->stop();
        return;
    }
    serverStatus = UNKNOWN;
    serverStatusLastUpdate = std::chrono::steady_clock::now();
    maxPlayercount = -1;
    playercount = -1;
    players = new std::list<std::string>();

    if(read_fd.is_open()){
        reader = std::thread([this](){ 
            while(read_fd.good())
                refresh();
            this->server->stop();
        });
    }else {
        std::cerr << "NON-FATAL: Failed to open fifo to get status from server.\n";
    }
}

idleDaemonIPC::~idleDaemonIPC(){
    if(reader.joinable())
        reader.join();
    write_fd.close();
    if(read_fd.is_open())
        read_fd.close();
    delete players;
}

void idleDaemonIPC::refresh(){
    std::string in;
    getline(read_fd, in);
    int space = in.find(' ');
    if(space == std::string::npos){
        std::cerr << "Each line must have at least 2 words separated by a space. Received:\t" << in << std::endl;
        return;
    }
    std::string word1 = in.substr(0, space);
    std::string word2 = in.substr(space+1,in.length());
    if(word1 == "STATUS"){
        //std::cout << "DEBUG: status received\n";
        if(word2 == "OFFLINE")
            serverStatus = OFFLINE;
        else if(word2 == "ONLINE")
            serverStatus = ONLINE;
        else if(word2 == "STARTING")
            serverStatus = STARTING;
        else if(word2 == "STOPPING")
            serverStatus = STOPPING;
        else{
            std::cerr << "STATUS value is not OFFLINE|ONLINE|STARTING|STOPPING. Received:\t" << word2 << std::endl;
            serverStatus = UNKNOWN;
        }
        serverStatusLastUpdate = std::chrono::steady_clock::now();
    }else if(word1 == "PLAYERS"){
        std::list<std::string>* playersInfo = new std::list<std::string>();
        for(int it = 0; space != std::string::npos; it = space+1){
            space = word2.substr(it, word2.length()).find(' ');
            playersInfo->push_back(word2.substr(it, (space == std::string::npos) ? word2.length() : space));
        }
        if(playersInfo->size() < 2){
            std::cerr << "PLAYERS must have at least 2 following words\n";
            return;
        }
        playercount = std::stoi(playersInfo->front());
        playersInfo->pop_front();
        maxPlayercount = std::stoi(playersInfo->front());
        playersInfo->pop_front();
        delete players;
        players = playersInfo;
    }
}

std::string idleDaemonIPC::getServerStatus() {
    if((serverStatus == STARTING || serverStatus == STOPPING || serverStatus == UNKNOWN) 
        && std::chrono::steady_clock::now() - serverStatusLastUpdate > std::chrono::seconds(20)){
        write_fd << "QUERY STATUS\n";
        write_fd.flush();
        serverStatusLastUpdate = std::chrono::steady_clock::now();//don't ask again until 20 seconds have passed
        //std::cout << "DEBUG: asked for update on STATUS\n";
    }
    switch(serverStatus){
        case OFFLINE:
            return "OFFLINE";
        case ONLINE:
            return "ONLINE";
        case STARTING:
            return "STARTING";
        case STOPPING:
            return "STOPPING";
        default:
            return "UNKNOWN";
    }
}

int idleDaemonIPC::getPlayercount() const {
    return playercount;
}

int idleDaemonIPC::getMaxPlayercount() const {
    return maxPlayercount;
}

std::list<std::string> idleDaemonIPC::getPlayers() const {
    return std::list<std::string>(*players);
}

json idleDaemonIPC::getFullStatus(){
    json j;
    j["status"] = getServerStatus();
    j["playercount"] = getPlayercount();
    j["maxPlayercount"] = getMaxPlayercount();
    j["players"] = getPlayers();
    return j;
}

void idleDaemonIPC::wake(){
    write_fd << "WAKE\n";
    write_fd.flush();
}