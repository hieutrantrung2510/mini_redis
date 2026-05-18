#pragma once

class TcpServer {
private:
    int port;
    int server_fd;
public:
    explicit TcpServer(int port);
    ~TcpServer();

    bool start();
    void run();
};
