#ifndef NET_SOCKET_HPP
#define NET_SOCKET_HPP

#include "io/FdHandle.hpp"
#include <stdexcept>
#include <string>

namespace net {

class Socket {
  public:
    class Error : public std::runtime_error {
      public:
        explicit Error(const std::string &msg) : std::runtime_error(msg) {}
    };

    explicit Socket(int fd);

    int fd() const;

    // Opt-in: the caller decides blocking vs non-blocking. The library never
    // forces a mode, so a Socket stays whatever the adopted fd already was.
    void setNonBlocking();

  private:
    io::FdHandle _handle;
};

} // namespace net

#endif
