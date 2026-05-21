#include "mini_redis/TcpServer.h"
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sys/event.h>

#define MAX_EVENTS 64

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

void TcpServer::acceptNewClients() {
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }

            std::cerr << "accept failed: " << strerror(errno) << "\n";
            break;
        }

        if (!setNonBlocking(client_fd)) {
            std::cerr << "Failed to set client non-blocking: "
                      << strerror(errno) << "\n";
            close(client_fd);
            continue;
        }

        clients.emplace(client_fd, Client{client_fd, "", ""});

        struct kevent change;
        EV_SET(
            &change,
            client_fd,
            EVFILT_READ,
            EV_ADD,
            0,
            0,
            nullptr
        );

        if (kevent(kqueue_fd, &change, 1, nullptr, 0, nullptr) < 0) {
            std::cerr << "Failed to register client fd "
                      << client_fd << ": "
                      << strerror(errno) << "\n";

            close(client_fd);
            clients.erase(client_fd);
            continue;
        }

        std::cout << "Client connected, fd = " << client_fd << "\n";
    }
}

void TcpServer::readFromClient(int client_fd) {
    auto it = clients.find(client_fd);

    if (it == clients.end()) {
        return;
    }

    Client& client = it->second;

    char buffer[1024];

    while (true) {
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            client.read_buffer.append(buffer, bytes_read);

            std::cout << "Received from fd " << client_fd
                      << ", " << bytes_read << " bytes:\n";

            std::cout.write(buffer, bytes_read);
            std::cout << "\n";

            const char* response = "+PONG\r\n";
            ssize_t bytes_written = write(client_fd, response, strlen(response));

            if (bytes_written < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }

                std::cerr << "write failed: " << strerror(errno) << "\n";
                closeClient(client_fd);
                return;
            }
        } else if (bytes_read == 0) {
            std::cout << "Client closed connection, fd = "
                      << client_fd << "\n";
            closeClient(client_fd);
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }

            std::cerr << "read failed: " << strerror(errno) << "\n";
            closeClient(client_fd);
            return;
        }
    }
}

void TcpServer::closeClient(int client_fd) {
    struct kevent change;

    EV_SET(
        &change,
        client_fd,
        EVFILT_READ,
        EV_DELETE,
        0,
        0,
        nullptr
    );

    kevent(kqueue_fd, &change, 1, nullptr, 0, nullptr);

    clients.erase(client_fd);
    close(client_fd);

    std::cout << "Closed client fd = " << client_fd << "\n";
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

    struct kevent events[MAX_EVENTS];

    while (true) {
        int event_count = kevent(
            kqueue_fd,
            nullptr,
            0,
            events,
            MAX_EVENTS,
            nullptr
        );

        if (event_count < 0) {
            std::cerr << "kevent wait failed: "
                      << strerror(errno) << "\n";
            continue;
        }

        for (int i = 0; i < event_count; ++i) {
            int fd = static_cast<int>(events[i].ident);

            if (fd == server_fd) {
                acceptNewClients();
            } else {
                readFromClient(fd);
            }
        }
    }
}