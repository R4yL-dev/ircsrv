#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "Error.hpp"
#include "io/Epoll.hpp"
#include "io/FdHandle.hpp"
#include "net/Socket.hpp"
#include "net/tcp.hpp"
#include "signals.hpp"

#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
const std::size_t MAX_SEND_QUEUE = 1024 * 1024; // 1 MiB per-client send backlog
} // namespace

Server::Server(const Config &cfg)
    : _config(cfg), _listen(net::tcpListen(cfg.ip(), cfg.port())),
      _signalFd(signals::shutdownMask()), _listenerPaused(false) {}

Server::~Server() {
    for (std::map<int, Client *>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        delete it->second;
    }
}

void Server::run() {
    _epoll.add(_listen.fd(), true, false);
    _epoll.add(_signalFd.fd(), true, false);

    for (;;) {
        std::vector<io::Event> events = _epoll.wait();

        for (std::size_t i = 0; i < events.size(); ++i) {
            const io::Event &ev = events[i];

            if (ev.fd == _signalFd.fd()) {
                _signalFd.readSignal();
                return; // graceful shutdown; ~Server frees the clients
            }

            try {
                if (ev.fd == _listen.fd()) {
                    acceptClients();
                } else {
                    serviceClient(ev);
                }
            } catch (const FatalError &) {
                throw;
            } catch (const std::runtime_error &e) {
                std::cerr << "client error (fd=" << ev.fd << "): " << e.what()
                          << "\n";
                markForClose(ev.fd, "internal error");
            }
        }

        reconcileWriteInterest();
        reapClosing();
    }
}

void Server::acceptClients() {
    for (;;) {
        int rawFd = -1;
        net::AcceptStatus st = net::tcpAccept(_listen.fd(), rawFd);
        if (st == net::AcceptWouldBlock) {
            break; // backlog drained
        }
        if (st == net::AcceptThrottled) {
            _epoll.setReadable(_listen.fd(), false);
            _listenerPaused = true;
            std::cerr << "accept throttled (out of fds): listener paused\n";
            break;
        }

        io::FdHandle handle(rawFd);
        std::auto_ptr<Client> client(new Client(rawFd, MAX_SEND_QUEUE));
        handle.release();         // Client now solely owns the fd
        client->setNonBlocking(); // throws => auto_ptr closes the fd exactly once

        _epoll.add(rawFd, true, false);
        _clients[rawFd] = client.get();
        client.release();

        std::cout << "Client connected (fd=" << rawFd << ")\n";
    }
}

void Server::serviceClient(const io::Event &ev) {
    if (_closing.find(ev.fd) != _closing.end()) {
        return; // already scheduled for removal this iteration
    }
    // HUP/ERR are folded into the read path: recv reports the verdict and any
    // final bytes get drained first.
    if (ev.readable || ev.hup || ev.error) {
        if (!handleReadable(ev.fd)) {
            return; // client was marked for close
        }
    }
    if (ev.writable) {
        handleWritable(ev.fd);
    }
}

bool Server::handleReadable(int fd) {
    std::map<int, Client *>::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return false;
    }
    Client *client = it->second;

    net::IoStatus st = client->receive();
    if (st == net::Closed) {
        markForClose(fd, client->closeReason());
        return false;
    }

    std::string msg;
    while (client->getNextMessage(msg)) {
        dispatch(*client, msg);
        if (_closing.find(fd) != _closing.end()) {
            return false; // dispatch closed this very client
        }
    }
    return true;
}

void Server::handleWritable(int fd) {
    std::map<int, Client *>::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return;
    }
    Client *client = it->second;

    if (client->flush() == net::Closed) {
        markForClose(fd, client->closeReason());
        return;
    }
    _dirtyOut.insert(fd); // reconcile EPOLLOUT (disarm once drained)
}

void Server::dispatch(Client &client, const std::string &msg) {
    std::string reply = msg + "\r\n"; // echo for now; IRC parsing goes here
    sendTo(client, reply.data(), reply.size());
}

void Server::sendTo(Client &client, const char *data, std::size_t len) {
    if (client.queueSend(data, len) == net::Closed) {
        markForClose(client.fd(), client.closeReason());
        return;
    }
    _dirtyOut.insert(client.fd());
}

void Server::markForClose(int fd, const char *reason) {
    if (_closing.find(fd) == _closing.end()) {
        _closing[fd] = (reason && *reason) ? reason : "closed";
    }
}

void Server::reconcileWriteInterest() {
    for (std::set<int>::iterator it = _dirtyOut.begin(); it != _dirtyOut.end();
         ++it) {
        int fd = *it;
        if (_closing.find(fd) != _closing.end()) {
            continue; // about to be reaped
        }
        std::map<int, Client *>::iterator c = _clients.find(fd);
        if (c == _clients.end()) {
            continue;
        }
        _epoll.setWritable(fd, c->second->hasPendingOutput());
    }
    _dirtyOut.clear();
}

void Server::reapClosing() {
    if (_closing.empty()) {
        return;
    }
    bool freedFd = false;
    for (std::map<int, std::string>::iterator it = _closing.begin();
         it != _closing.end(); ++it) {
        int fd = it->first;
        std::map<int, Client *>::iterator c = _clients.find(fd);
        if (c == _clients.end()) {
            continue; // not a live client (e.g. the listener fd)
        }
        _epoll.remove(fd);
        _dirtyOut.erase(fd);
        std::cout << "Client disconnected (fd=" << fd
                  << ", reason=" << it->second << ")\n";
        delete c->second;
        _clients.erase(c);
        freedFd = true;
    }
    _closing.clear();

    if (freedFd && _listenerPaused) {
        _epoll.setReadable(_listen.fd(), true);
        _listenerPaused = false;
        std::cerr << "listener resumed\n";
    }
}
