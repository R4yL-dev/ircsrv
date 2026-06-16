#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Config.hpp"
#include "io/Epoll.hpp"
#include "io/SignalFd.hpp"
#include "net/Socket.hpp"

#include <cstddef>
#include <map>
#include <set>
#include <string>

class Server {
  public:
    explicit Server(const Config &cfg);
    ~Server();

    void run();

  private:
    void acceptClients();
    void serviceClient(const io::Event &ev);
    bool handleReadable(int fd); // false => client was marked for close
    void handleWritable(int fd);
    void dispatch(Client &client, const std::string &msg);
    void sendTo(Client &client, const char *data, std::size_t len);

    void markForClose(int fd, const char *reason);
    void reconcileWriteInterest(); // end-of-iteration: arm/disarm EPOLLOUT
    void reapClosing();            // end-of-iteration: the only delete site

    const Config _config;
    net::Socket _listen;
    io::Epoll _epoll;
    io::SignalFd _signalFd;
    std::map<int, Client *> _clients;
    std::set<int> _dirtyOut;             // clients written to this iteration
    std::map<int, std::string> _closing; // fd -> reason, reaped at iteration end
    bool _listenerPaused;                // accept throttled on fd exhaustion
};

#endif
