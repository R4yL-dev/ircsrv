#ifndef IO_SIGNALFD_HPP
#define IO_SIGNALFD_HPP

#include "io/FdHandle.hpp"

#include <csignal>
#include <stdexcept>
#include <string>

namespace io {

class SignalFd {
  public:
    class Error : public std::runtime_error {
      public:
        explicit Error(const std::string &msg) : std::runtime_error(msg) {}
    };

    explicit SignalFd(const sigset_t &mask);
    int fd() const;
    int readSignal();

  private:
    io::FdHandle _handle;
};

} // namespace io

#endif
