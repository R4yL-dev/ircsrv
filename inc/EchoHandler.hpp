#ifndef ECHO_HANDLER_HPP
#define ECHO_HANDLER_HPP

#include "IMessageHandler.hpp"

#include <string>

// Trivial IMessageHandler that echoes each received line back to its sender.
// Exercises the whole transport seam (framing, send queue, lingering close)
// with no protocol logic -- the placeholder until the IRC handler exists.
class EchoHandler : public IMessageHandler {
  public:
    virtual void onConnect(IServerOps &ops, int fd);
    virtual void onMessage(IServerOps &ops, int fd, const std::string &line);
    virtual void onDisconnect(IServerOps &ops, int fd, const char *reason);
    virtual void onTick(IServerOps &ops, long nowMs);
};

#endif
