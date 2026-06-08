#ifndef NET_SOCKET_HPP
#define NET_SOCKET_HPP

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
    ~Socket();

    int fd() const;

  private:
    int _fd;

    Socket(const Socket &);
    Socket &operator=(const Socket &);
};

} // namespace net

#endif
