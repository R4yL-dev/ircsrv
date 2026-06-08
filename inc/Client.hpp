#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "net/Connection.hpp"
#include <cstddef>
#include <sys/types.h>

class Client {
  public:
    explicit Client(int fd);

    int fd() const;
    ssize_t recv(char *buf, size_t len);

  private:
    net::Connection _conn;
};

#endif
