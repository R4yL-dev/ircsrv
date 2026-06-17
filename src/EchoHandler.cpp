#include "EchoHandler.hpp"
#include "IServerOps.hpp"

#include <string>

void EchoHandler::onConnect(IServerOps &, int) {}

void EchoHandler::onMessage(IServerOps &ops, int fd, const std::string &line) {
    std::string reply = line + "\r\n";
    ops.send(fd, reply.data(), reply.size());
}

void EchoHandler::onDisconnect(IServerOps &, int, const char *) {}

void EchoHandler::onTick(IServerOps &, long) {}
