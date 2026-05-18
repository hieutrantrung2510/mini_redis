#include "mini_redis/TcpServer.h"
#include <unistd.h>

TcpServer::TcpServer(int port) : port(port), server_fd(-1) {}

TcpServer::~TcpServer() {
    if (server_fd != -1) {
        close(server_fd);
    }
}
