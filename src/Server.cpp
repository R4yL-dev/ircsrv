#include "Server.hpp"
#include "Config.hpp"
#include "io/Epoll.hpp"
#include "net/Socket.hpp"
#include "net/tcp.hpp"

#include <iostream>
#include <vector>

#include <unistd.h>

Server::Server(const Config &cfg)
    : _config(cfg), _listen(net::tcpListen(cfg.ip(), cfg.port())) {}

void Server::run() {
    _epoll.add(_listen.fd());

    while (true) {
        std::vector<io::Event> events = _epoll.wait();

        for (size_t i = 0; i < events.size(); ++i) {
            if (events[i].fd == _listen.fd()) {
                int clientFd = net::tcpAccept(_listen.fd());
                std::cout << "Client connected (fd=" << clientFd << ")\n";
                close(clientFd);
            }
        }
    }
}
