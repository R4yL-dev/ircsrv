#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "framing/LineFramer.hpp"
#include "net/Connection.hpp"
#include "net/IoStatus.hpp"

#include <cstddef>
#include <string>

class Client {
  public:
    Client(int fd, std::size_t maxSendQueue);

    int fd() const;
    void setNonBlocking();

    net::IoStatus receive();               // one recv into the inbound buffer
    bool getNextMessage(std::string &out); // next framed line, if available

    net::IoStatus queueSend(const char *data, std::size_t len);
    net::IoStatus flush();
    bool hasPendingOutput() const;

    const char *closeReason() const;

  private:
    net::Connection _conn;
    framing::LineFramer _framer;
};

#endif
