#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "LineBuffer.hpp"
#include "net/Connection.hpp"

#include <cstddef>
#include <string>

class Client {
  public:
    explicit Client(int fd);

    int fd() const;
    bool receive();
    bool send(const char *data, std::size_t len);
    bool getNextMessage(std::string &out);

  private:
    net::Connection _conn;
    LineBuffer _inbuf;
};

#endif
