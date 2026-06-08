#include "Server.hpp"
#include "Config.hpp"
#include "net/Socket.hpp"
#include "net/tcp.hpp"

#include <iostream>

#include <unistd.h>

Server::Server(const Config &cfg)
    : _config(cfg), _listen(net::tcpListen(cfg.ip(), cfg.port())) {}

void Server::run() {
    while (true) {
        int clientFd = net::tcpAccept(_listen.fd());
        std::cout << "Client connected (fd=" << clientFd << ")\n";
        close(clientFd);
    }
}
