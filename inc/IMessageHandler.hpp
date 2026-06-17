#ifndef IMESSAGE_HANDLER_HPP
#define IMESSAGE_HANDLER_HPP

#include "IServerOps.hpp"

#include <string>

// The protocol layer implements this; Server calls it at the three lifecycle
// points of a connection. Each callback receives an IServerOps& so the handler
// can send to / disconnect any client -- e.g. relay a PRIVMSG, or broadcast a
// QUIT to channel peers when a client leaves. ops is passed per-call (not stored
// in the handler) to avoid a construction cycle: Server needs the handler, and
// the handler would otherwise need the Server.
class IMessageHandler {
  public:
    virtual ~IMessageHandler() {}

    virtual void onConnect(IServerOps &ops, int fd) = 0;
    virtual void onMessage(IServerOps &ops, int fd, const std::string &line) = 0;
    virtual void onDisconnect(IServerOps &ops, int fd, const char *reason) = 0;

    // Periodic time hook (monotonic milliseconds, see io::monotonicMs). Called at
    // a coarse fixed cadence even when no I/O happens; the handler compares nowMs
    // against its own deadlines (ping timeout, registration timeout, ...).
    virtual void onTick(IServerOps &ops, long nowMs) = 0;
};

#endif
