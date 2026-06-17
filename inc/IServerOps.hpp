#ifndef ISERVER_OPS_HPP
#define ISERVER_OPS_HPP

#include <cstddef>

// Operations the transport layer (Server) exposes to a message handler, so the
// handler can act on ANY connection -- not just the one that triggered the
// current callback. Server implements this and passes itself to every
// IMessageHandler callback. Kept free of protocol concepts so the server layer
// stays extractable as a generic socket library.
class IServerOps {
  public:
    virtual ~IServerOps() {}

    // Queue bytes to a client by fd. No-op if the fd is unknown (e.g. the client
    // already disconnected this iteration).
    virtual void send(int fd, const char *data, std::size_t len) = 0;

    // Schedule a client for disconnection, with a diagnostic reason.
    virtual void disconnect(int fd, const char *reason) = 0;
};

#endif
