#pragma once

class TcpServer {
private:
    int port;
    int server_fd;

    void handleClient(int client_fd);

public:
    explicit TcpServer(int port);
    ~TcpServer();

    bool start();
    void run();
};
