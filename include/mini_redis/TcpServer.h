#pragma once

#include "mini_redis/Client.h"

class TcpServer {
private:
    int port;
    int server_fd;
    int kqueue_fd;

    std::unordered_map<int, Client> clients;

    bool setNonBlocking(int fd) {};
    void acceptNewClients();
    void readFromClient(int client_fd);
    void closeClient(int client_fd);
    
public:
    explicit TcpServer(int port);
    ~TcpServer();

    bool start();
    void run();
};
