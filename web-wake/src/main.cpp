#include "httpsServer.hpp"
#include <iomanip>
#include <sstream>
#include <json.hpp>
#include <sys/random.h>
using json = nlohmann::json;

//in case config.json is missing it will be regenerated
const json defaultConfig = {
    {"listen host", "0.0.0.0"},
    {"listen port", 25566},
    {"token", "uninitialized"}
};
const std::string configPath = "config.json";

std::string genToken(){
    unsigned char randomBytes[32];
    int filled = 0;
    while(filled < sizeof(randomBytes)){
        int n = getrandom(randomBytes+filled, sizeof(randomBytes)-filled, 0);
        if(n < 0){
            if(errno != EINTR)
                throw std::system_error(errno, std::generic_category(), "getrandom");
        }else
            filled += n;
    }
    std::stringstream ss;
    ss << std::hex;
    for(int i = 0; i < sizeof(randomBytes); i++)
        ss << std::setw(2) << (int)randomBytes[i];
    return ss.str();
}

int main(int argc, char* argv[]){
    if(argc < 3){
        std::string thisname = (argv[0] == nullptr || strcmp(argv[0], "") == 0) ? "<this executable>" : argv[0];
        std::cerr << "Usage: " << thisname << " <cmds to @INAME@ idle Daemon fifo> <@GAME@ server status to " << thisname << " fifo>\n";
        return 1;
    }
    std::ifstream configFile(configPath);
    json config;
    if(!configFile){
        std::ofstream ofs(configPath);
        if(ofs.is_open()){
            std::cerr << "WARNING: No " << configPath << " was found, regenerating from defaults.\n";
            ofs << defaultConfig.dump(4) << '\n';
            ofs.close();
        }else 
            std::cerr << "ERROR: No config.json was found and it couldn't be regenerated, using defaults.\n";
        config = defaultConfig;
    }else{
        config = json::parse(configFile);
    }
    if(config["token"] == "uninitialized"){
        try{
            std::ofstream ofs(configPath, std::fstream::out | std::fstream::trunc);
            if(ofs.is_open()){
                config["token"] = genToken();
                ofs << defaultConfig.dump(4) << '\n';
                ofs.close();
            }else
                std::cerr << "WARNING: Failed to open " << configPath << " to write the generated token, it will remain as \"uninitialized\".\n";
        }catch(const std::exception &e){
            std::cerr << "WARNING: Failed to generate and write token, left uninitialized.\n";
            std::cerr << "\tCaught exception: " << e.what() << std::endl;
        }
    }

    httpsServer server(config["listen host"], config["listen port"], config["token"], argv[1], argv[2]);
    
    return server.run();
}