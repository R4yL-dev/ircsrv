#ifndef IO_EPOLL_HPP
#define IO_EPOLL_HPP

#include "io/FdHandle.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace io {

struct Event {
    int fd;
};

class Epoll {
  public:
    class Error : public std::runtime_error {
      public:
        explicit Error(const std::string &msg) : std::runtime_error(msg) {}
    };

    Epoll();

    void add(int fd);
    void remove(int fd);
    std::vector<Event> wait();

  private:
    io::FdHandle _handle;
};

} // namespace io

#endif
