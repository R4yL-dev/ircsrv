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
    ssize_t recv(char *buf, std::size_t len);
    bool send(const char *data, std::size_t len);

  private:
    Socket _socket;
};

} // namespace net

#endif
