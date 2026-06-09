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
#include <string>
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

        for (std::size_t i = 0; i < events.size(); ++i) {
            if (events[i].fd == _listen.fd()) {
                try {
                    acceptClient();
                } catch (const net::Socket::Error &e) {
                    std::cerr << "accept failed: " << e.what() << "\n";
                }
            } else {
                int fd = events[i].fd;

                std::map<int, Client *>::iterator it = _clients.find(fd);
                if (it == _clients.end()) {
                    continue;
                }
                Client *client = it->second;

                if (!client->receive()) {
                    disconnectClient(fd);
                } else {
                    std::string msg;
                    bool alive = true;
                    while (alive && client->getNextMessage(msg)) {
                        std::string reply = msg + "\r\n";
                        if (!client->send(reply.c_str(), reply.size())) {
                            disconnectClient(fd);
                            alive = false;
                        }
                    }
                }
            }
        }
    }
}

void Server::acceptClient() {
    int clientFd = net::tcpAccept(_listen.fd());
    _clients[clientFd] = new Client(clientFd);
    _epoll.add(clientFd);
    std::cout << "Client connected (fd=" << clientFd << ")\n";
}

void Server::disconnectClient(int fd) {
    _epoll.remove(fd);
    delete _clients[fd];
    _clients.erase(fd);
    std::cout << "Client disconnected (fd=" << fd << ")\n";
}