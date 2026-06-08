#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "net/Socket.hpp"

class Client {
  public:
    explicit Client(int fd);

    int fd() const;

  private:
    net::Socket _socket;
};

#endif
