#include "httpsServer.hpp"
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <json.hpp>
#include <string>
#include <sys/random.h>
#include <filesystem>
#include <system_error>
using json = nlohmann::json;

//in case config.json is missing these will be used
const json defaultConfig = {
    {"listen host", "0.0.0.0"},
    {"listen port", 25564}
};

#ifdef CONFIG_PATH
const std::string configPath = CONFIG_PATH;
#else
const std::string configPath = "config.json";//fallback
#endif
#ifdef TOKEN_PATH
const std::string tokenPath = TOKEN_PATH;
#else
const std::string tokenPath = ".web-wake-token";//fallback
#endif

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
    ss << std::hex << std::setfill('0');
    for(int i = 0; i < sizeof(randomBytes); i++)
        ss << std::setw(2) << (int)randomBytes[i];
    return ss.str();
}

void writeToken(std::string newToken){
    std::ofstream ofs(tokenPath, std::ios_base::out | std::ios_base::trunc);
    if(ofs.is_open()){
        ofs << newToken << std::endl;
        ofs.close();
        std::error_code ec;
        std::filesystem::permissions(tokenPath, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
            std::filesystem::perm_options::replace, ec);
        if(ec)
            std::cerr << "WARNING: Failed to set 600 permissions for token file: " << ec.message() << std::endl;
    }else
        std::cerr << "WARNING: Failed to open " << tokenPath << " for writing, using temporary token for this run:\n" << newToken << std::endl;
}

std::string getOrGenToken(){
    std::string token;
    if(!std::filesystem::exists(tokenPath)){//if it doesn't exist, generate it
        std::cerr << "Token file does not exists, generating...\n";
        token = genToken();
        writeToken(token);
    }else{//if it does exist, try to open it
        std::ifstream ifs(tokenPath);
        if(ifs.is_open()){
            std::getline(ifs, token);
            ifs.close();
            if(token.empty()){
                std::cerr << "ERROR: Failed to get token from " << tokenPath << ", attempting to regenerate...\n";
                token = genToken();
                writeToken(token);
            }
        }else{
            std::cerr << "ERROR: Token file exists but is unreadable, attempting to overwrite...\n";
            token = genToken();
            writeToken(token);
        }
    }
    return token;
}

int main(int argc, char* argv[]){
    if(argc < 3){
        std::string thisname = (argv[0] == nullptr || strcmp(argv[0], "") == 0) ? "<this executable>" : argv[0];
        std::cerr << "Usage: " << thisname << " <cmds to @INAME@ idle Daemon fifo> <@GAME@ server status to " << thisname << " fifo>\n";
        return 2;
    }
    /// Get config:
    std::ifstream configFile(configPath);
    json config;
    if(!configFile){
        std::cerr << "ERROR: No config.json was found, using defaults.\n";
        config = defaultConfig;
    }else{
        config = json::parse(configFile);
    }
    /// Get token:
    std::string token = getOrGenToken();
    /// Check for SSL certificate
    if(!std::filesystem::exists(certpath) || !std::filesystem::exists(keypath)){
        std::cerr << "FATAL: SSL Certificates are missing on paths " << certpath << " and " << keypath
            << "\nEither add your own or to generate self-signed ones run "
            << "\'openssl req -x509 -newkey rsa:2048 -days 365 -nodes -keyout " << keypath << " -out " << certpath << " -subj \"/CN=localhost\"\'\n";
        return 1;
    }

    httpsServer server(config["listen host"], config["listen port"], token, argv[1], argv[2]);
    
    std::cout << "Starting https server...\n";
    return server.run();
}