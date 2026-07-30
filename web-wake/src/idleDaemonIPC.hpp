#ifndef IDLE_DAEMON_IPC_HPP
#define IDLE_DAEMON_IPC_HPP

#include <string>

class idleDaemonIPC{
private:
    int write_fd;//fd of fifo to _send_ commands to the idle daemon
    int read_fd;//fd of fifo to _receive_ updates from the idle daemon
    bool connected;

    std::string serverStatus;
    int playercount;
public:
    idleDaemonIPC(const char* cmd_fifo_path, const char* status_fifo_path);
    ~idleDaemonIPC();

    bool bad() const;
    void refresh();
    std::string getSeverStatus() const;
    int getPlayercount() const;

    void wake();
};

#endif // IDLE_DAEMON_IPC_HPP