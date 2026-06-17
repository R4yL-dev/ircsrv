#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Config.hpp"
#include "IMessageHandler.hpp"
#include "IServerOps.hpp"
#include "io/Epoll.hpp"
#include "io/SignalFd.hpp"
#include "net/Socket.hpp"

#include <cstddef>
#include <map>
#include <set>
#include <string>

class Server : public IServerOps {
  public:
    Server(const Config &cfg, IMessageHandler &handler);
    ~Server();

    void run();

    // IServerOps: the handler may target any client, not just the current one.
    virtual void send(int fd, const char *data, std::size_t len);
    virtual void disconnect(int fd, const char *reason);

  private:
    void acceptClients();
    void serviceClient(const io::Event &ev);
    bool handleReadable(int fd); // false => client was marked for close
    void handleWritable(int fd);

    void markForClose(int fd, const char *reason);
    bool isClosing(int fd) const;  // hard close pending OR draining to close
    void reconcileWriteInterest(); // end-of-iteration: arm/disarm EPOLLOUT
    void reapClosing();            // end-of-iteration: the only delete site
    void maybeTick(long now);      // fire onTick at most once per tick interval

    const Config _config;
    IMessageHandler &_handler;
    net::Socket _listen;
    io::Epoll _epoll;
    io::SignalFd _signalFd;
    std::map<int, Client *> _clients;
    std::set<int> _dirtyOut;               // clients written to this iteration
    std::map<int, std::string> _closing;   // fd -> reason, hard close: reap now
    std::map<int, std::string> _lingering; // fd -> reason, reap once _out drains
    bool _listenerPaused;                  // accept throttled on fd exhaustion
    long _lastTick;                        // monotonic ms of the last onTick
};

#endif
