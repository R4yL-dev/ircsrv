#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "Error.hpp"
#include "io/Epoll.hpp"
#include "net/Socket.hpp"
#include "net/tcp.hpp"
#include "signals.hpp"

#include <cstddef>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

Server::Server(const Config &cfg)
    : _config(cfg), _listen(net::tcpListen(cfg.ip(), cfg.port())),
      _signalFd(signals::shutdownMask()) {}

Server::~Server() {
    for (std::map<int, Client *>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        delete it->second;
    }
}

void Server::run() {
    _epoll.add(_listen.fd());
    _epoll.add(_signalFd.fd());

    while (true) {
        std::vector<io::Event> events = _epoll.wait();

        for (std::size_t i = 0; i < events.size(); ++i) {
            int fd = events[i].fd;

            if (fd == _signalFd.fd()) {
                _signalFd.readSignal();
                return;
            }

            try {
                if (fd == _listen.fd()) {
                    acceptClient();
                } else {
                    handleClientData(fd);
                }
            } catch (const FatalError &) {
                throw;
            } catch (const std::runtime_error &e) {
                std::cerr << "client error (fd=" << fd << "): " << e.what()
                          << "\n";
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

void Server::handleClientData(int fd) {
    std::map<int, Client *>::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return;
    }
    Client *client = it->second;

    if (!client->receive()) {
        disconnectClient(fd);
        return;
    }
    std::string msg;
    while (client->getNextMessage(msg)) {
        std::string reply = msg + "\r\n";
        if (!client->send(reply.c_str(), reply.size())) {
            disconnectClient(fd);
            return;
        }
    }
}