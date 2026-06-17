#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "Error.hpp"
#include "io/Clock.hpp"
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
const int TICK_MS = 1000; // coarse cadence of the onTick time hook (ms)
} // namespace

Server::Server(const Config &cfg, IMessageHandler &handler)
    : _config(cfg), _handler(handler),
      _listen(net::tcpListen(cfg.ip(), cfg.port())),
      _signalFd(signals::shutdownMask()), _listenerPaused(false), _lastTick(0) {}

Server::~Server() {
    for (std::map<int, Client *>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        delete it->second;
    }
}

void Server::run() {
    _epoll.add(_listen.fd(), true, false);
    _epoll.add(_signalFd.fd(), true, false);
    _lastTick = io::monotonicMs();

    for (;;) {
        std::vector<io::Event> events = _epoll.wait(TICK_MS);

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

        maybeTick(io::monotonicMs());
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
        handle.release(); // Client now solely owns the fd
        client
            ->setNonBlocking(); // throws => auto_ptr closes the fd exactly once

        _epoll.add(rawFd, true, false);
        _clients[rawFd] = client.get();
        client.release();

        std::cout << "Client connected (fd=" << rawFd << ")\n";
        _handler.onConnect(*this, rawFd);
    }
}

void Server::serviceClient(const io::Event &ev) {
    if (_closing.find(ev.fd) != _closing.end()) {
        return; // hard close pending: ignore entirely this iteration
    }
    bool lingering = _lingering.find(ev.fd) != _lingering.end();

    // Peer drops while we're draining its farewell: give up flushing, reap now.
    if (lingering && (ev.hup || ev.error)) {
        _lingering.erase(ev.fd);
        markForClose(ev.fd, "peer closed");
        return;
    }
    // Reads are suppressed once we've decided to close gracefully. Otherwise
    // HUP/ERR fold into the read path: recv reports the verdict and any final
    // bytes get drained first.
    if (!lingering && (ev.readable || ev.hup || ev.error)) {
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
        _handler.onMessage(*this, fd, msg);
        if (isClosing(fd)) {
            return false; // the handler closed this client (hard or graceful)
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
        _lingering.erase(fd);
        markForClose(fd, client->closeReason());
        return;
    }
    _dirtyOut.insert(fd); // reconcile EPOLLOUT (disarm once drained)

    // A graceful close whose farewell has now fully gone out: reap it via the
    // single delete site by promoting it to a hard close.
    std::map<int, std::string>::iterator l = _lingering.find(fd);
    if (l != _lingering.end() && !client->hasPendingOutput()) {
        std::string reason = l->second;
        _lingering.erase(l);
        markForClose(fd, reason.c_str());
    }
}

void Server::send(int fd, const char *data, std::size_t len) {
    std::map<int, Client *>::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return; // unknown fd: client already gone this iteration
    }
    Client *client = it->second;
    if (client->queueSend(data, len) == net::Closed) {
        markForClose(fd, client->closeReason());
        return;
    }
    _dirtyOut.insert(fd);
}

void Server::disconnect(int fd, const char *reason) {
    std::map<int, Client *>::iterator it = _clients.find(fd);
    if (it == _clients.end() || _closing.find(fd) != _closing.end()) {
        return; // unknown fd, or a hard close already won
    }
    const char *r = (reason && *reason) ? reason : "closed";
    if (!it->second->hasPendingOutput()) {
        markForClose(fd, r); // nothing to flush: reap this iteration
        return;
    }
    // Drain the farewell first. Stop reading the peer (we've decided to close),
    // and disarm read interest so a level-triggered EPOLLIN doesn't spin; keep
    // EPOLLOUT armed so the flush continues across iterations.
    _lingering[fd] = r;
    _epoll.setReadable(fd, false);
    _dirtyOut.insert(fd);
}

void Server::markForClose(int fd, const char *reason) {
    if (_closing.find(fd) == _closing.end()) {
        _closing[fd] = (reason && *reason) ? reason : "closed";
    }
}

bool Server::isClosing(int fd) const {
    return _closing.find(fd) != _closing.end() ||
           _lingering.find(fd) != _lingering.end();
}

void Server::maybeTick(long now) {
    // Throttle: at most one tick per interval, however many I/O wakeups occurred.
    if (now - _lastTick < TICK_MS) {
        return;
    }
    _lastTick = now;
    _handler.onTick(*this, now);
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
    bool freedFd = false;
    // Drain rather than iterate-then-clear: onDisconnect may itself request more
    // closes (e.g. a cascading send failure), and those must be reaped this turn
    // too. No accept() runs between here and the next loop, so a freed fd cannot
    // be reused mid-drain.
    while (!_closing.empty()) {
        std::map<int, std::string>::iterator it = _closing.begin();
        int fd = it->first;
        std::string reason = it->second;
        _closing.erase(it); // remove first, so re-closing this fd is a no-op

        std::map<int, Client *>::iterator c = _clients.find(fd);
        if (c == _clients.end()) {
            continue; // not a live client (e.g. the listener fd)
        }
        _handler.onDisconnect(*this, fd, reason.c_str());
        _epoll.remove(fd);
        _dirtyOut.erase(fd);
        _lingering.erase(fd);
        std::cout << "Client disconnected (fd=" << fd << ", reason=" << reason
                  << ")\n";
        delete c->second;
        _clients.erase(c);
        freedFd = true;
    }

    if (freedFd && _listenerPaused) {
        _epoll.setReadable(_listen.fd(), true);
        _listenerPaused = false;
        std::cerr << "listener resumed\n";
    }
}
