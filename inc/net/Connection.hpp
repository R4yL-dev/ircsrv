#ifndef NET_CONNECTION_HPP
#define NET_CONNECTION_HPP

#include "net/Socket.hpp"

#include <cstddef>

#include <sys/types.h>

namespace net {

class Connection {
  public:
    explicit Connection(int fd);

    int fd() const;
    ssize_t recv(char *buf, size_t len);

  private:
    Socket _socket;
};

} // namespace net

#endif
