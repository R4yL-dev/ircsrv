#ifndef IO_EPOLL_HPP
#define IO_EPOLL_HPP

#include "io/FdHandle.hpp"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace io {

struct Event {
    int fd;
    bool readable; // EPOLLIN
    bool writable; // EPOLLOUT
    bool error;    // EPOLLERR  (always reported, never requested)
    bool hup;      // EPOLLHUP  (always reported, never requested)
};

// Thin epoll(7) wrapper. Tracks each fd's interest so a write-readiness toggle
// is a no-op when unchanged and preserves the read interest. The API speaks in
// read/write booleans, never in EPOLL* constants. EPOLLERR/EPOLLHUP need not be
// requested -- epoll always reports them.
class Epoll {
  public:
    class Error : public std::runtime_error {
      public:
        explicit Error(const std::string &msg) : std::runtime_error(msg) {}
    };

    Epoll();

    void add(int fd, bool read, bool write);
    void setReadable(int fd, bool on);
    void setWritable(int fd, bool on);
    void remove(int fd);

    std::vector<Event> wait();

  private:
    struct Interest {
        bool read;
        bool write;
    };

    void applyInterest(int fd, const Interest &want);

    io::FdHandle _handle;
    std::map<int, Interest> _interest;
};

} // namespace io

#endif
