#pragma once

#include "mini_redis/Client.h"
#include <unordered_map>

class TcpServer {
private:
    int port;
    int server_fd;
    int kqueue_fd;

    std::unordered_map<int, Client> clients;

    bool setNonBlocking(int fd);
    void acceptNewClients();
    void readFromClient(int client_fd);
    void closeClient(int client_fd);
    void enableWriteEvent(int client_fd);
    void disableWriteEvent(int client_fd);
    void writeToClient(int client_fd);
    
public:
    explicit TcpServer(int port);
    ~TcpServer();

    bool start();
    void run();
};
