#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config.hpp"
#include "io/Epoll.hpp"
#include "net/Socket.hpp"

#include <stdexcept>
#include <string>

class Server {
  public:
    class Error : public std::runtime_error {
      public:
        explicit Error(const std::string &msg) : std::runtime_error(msg) {}
    };

    explicit Server(const Config &cfg);

    void run();

  private:
    const Config _config;
    net::Socket _listen;
    io::Epoll _epoll;
};

#endif
