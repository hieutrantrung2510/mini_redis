#include "mini_redis/TcpServer.h"
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sys/event.h>

TcpServer::TcpServer(int port) : port(port), server_fd(-1), kqueue_fd(-1) {}

TcpServer::~TcpServer() {
    if (server_fd != -1) {
        close(server_fd);
    }

    if (kqueue_fd != -1) {
        close(kqueue_fd);
    }
}

bool TcpServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        std::cerr << "fcntl F_GETFL failed: " << strerror(errno) << "\n";
        return false;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl F_SETFL failed: " << strerror(errno) << "\n";
        return false;
    }

    return true;
}

bool TcpServer::start() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        std::cerr << "Failed to create socket server \n";
        return false;
    }

    if (!setNonBlocking(server_fd)) {
        std::cerr << "Failed to set TCP server non-blocking: " << strerror(errno) << '\n';
        return false;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed: " << strerror(errno) << "\n";
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "Failed to bind to port " << port
         << ": " << strerror(errno) << "\n";
        return false;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Failed to listen : " << strerror(errno) << '\n';
        return false;
    }

    std::cout << "Mini Redis listening on port " << port << "\n";
    return true;
}

void TcpServer::run() {
    kqueue_fd = kqueue();

    if (kqueue_fd < 0) {
        std::cerr << "Failed to create kqueue: "
                  << strerror(errno) << "\n";
        return;
    }

    struct kevent change;

    EV_SET(
        &change,
        server_fd,
        EVFILT_READ,
        EV_ADD,
        0,
        0,
        nullptr
    );

    if (kevent(kqueue_fd, &change, 1, nullptr, 0, nullptr) < 0) {
        std::cerr << "Failed to register server_fd: "
                  << strerror(errno) << "\n";
        return;
    }

    struct kevent events[64];

    int event_count = 

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd < 0) {
            std::cerr << "Failed to accept client\n" << strerror(errno) << "\n";;
            continue;
        }

        Client newClient;
        newClient.fd = client_fd;
        
        if (!setNonBlocking(newClient.fd)) {
            std::cerr << "Failed to set TCP client non-blocking: " << strerror(errno) << '\n';
        }

        std::cout << "Client connected\n";

        // handleClient(newClient);

        std::cout << "Client disconnected\n";
    }
}

// void TcpServer::handleClient(Client client) {
//     while (true) {
//         char buffer[1024] = {};
//         ssize_t bytes_read = read(client.fd, buffer, sizeof(buffer) - 1);

//         if (bytes_read == 0) {
//             std::cout << "Client closed connection\n";
//             break;
//         } else if (bytes_read == -1){
//             std::cerr << "Read failed: " << strerror(errno) << "\n";
//             break;
//         }

//         std::cout << "Received " << bytes_read << " bytes:\n";
//         std::cout << buffer << "\n";

//         const char* response = "+PONG\r\n";
//         ssize_t bytes_written = write(client_fd, response, strlen(response));

//         if (bytes_written < 0) {
//             std::cerr << "Write failed: " << strerror(errno) << "\n";
//             break;
//         }
//     }

//     close(client_fd);
// }
