#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "io/Epoll.hpp"
#include "net/Socket.hpp"
#include "net/tcp.hpp"
#include "signals.hpp"

#include <cstddef>
#include <iostream>
#include <map>
#include <vector>

#include <sys/types.h>

namespace {
const std::size_t READ_BUFFER_SIZE = 512;
}

Server::Server(const Config &cfg)
    : _config(cfg), _listen(net::tcpListen(cfg.ip(), cfg.port())) {}

Server::~Server() {
    for (std::map<int, Client *>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        delete it->second;
    }
}

void Server::run() {
    _epoll.add(_listen.fd());

    while (!signals::stopRequested()) {
        std::vector<io::Event> events = _epoll.wait();

        for (size_t i = 0; i < events.size(); ++i) {
            if (events[i].fd == _listen.fd()) {
                int clientFd = net::tcpAccept(_listen.fd());
                std::cout << "Client connected (fd=" << clientFd << ")\n";
                _clients[clientFd] = new Client(clientFd);
                _epoll.add(clientFd);
            } else {
                int fd = events[i].fd;
                Client *client = _clients[fd];

                char buf[READ_BUFFER_SIZE];
                ssize_t n = client->recv(buf, sizeof(buf));

                if (n <= 0) {
                    _epoll.remove(fd);
                    delete _clients[fd];
                    _clients.erase(fd);
                    std::cout << "Client disconnected (fd=" << fd << ")\n";
                } else {
                    std::cout << "Received from fd=" << fd << ": ";
                    std::cout.write(buf, n);
                }
            }
        }
    }
}
