#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "io/Epoll.hpp"
#include "net/Socket.hpp"
#include "net/tcp.hpp"
#include "signals.hpp"

#include <iostream>
#include <map>
#include <vector>

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
            }
        }
    }
}
