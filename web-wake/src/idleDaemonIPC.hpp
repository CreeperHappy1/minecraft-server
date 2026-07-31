#ifndef IDLE_DAEMON_IPC_HPP
#define IDLE_DAEMON_IPC_HPP

#include <atomic>
#include <fstream>
#include <string>
#include <thread>
#include <httplib.h>

enum status {OFFLINE, ONLINE, STARTING, STOPPING, UNKNOWN};

class idleDaemonIPC{
private:
    std::ofstream write_fd;//fd of fifo to _send_ commands to the idle daemon
    std::ifstream read_fd;//fd of fifo to _receive_ updates from the idle daemon
    httplib::Server* server;

    std::atomic<status> serverStatus;
    std::atomic<int> playercount;
    std::atomic<int> maxPlayercount;
    std::atomic<std::list<std::string>*> players;

    void refresh();//blocking refresh from read_fd
    std::thread reader;
public:
    idleDaemonIPC(const char* cmd_fifo_path, const char* status_fifo_path, httplib::Server* server);
    ~idleDaemonIPC();

    std::string getServerStatus() const;
    int getPlayercount() const;

    void wake();
};

#endif // IDLE_DAEMON_IPC_HPP