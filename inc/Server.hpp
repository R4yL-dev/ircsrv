#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Config.hpp"
#include "io/Epoll.hpp"
#include "io/SignalFd.hpp"
#include "net/Socket.hpp"

#include <map>

class Server {
  public:
    explicit Server(const Config &cfg);
    ~Server();

    void run();

  private:
    void acceptClient();
    void disconnectClient(int fd);
    void handleClientData(int fd);

    const Config _config;
    net::Socket _listen;
    io::Epoll _epoll;
    io::SignalFd _signalFd;
    std::map<int, Client *> _clients;
};

#endif
