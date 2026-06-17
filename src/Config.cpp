#include "Config.hpp"
#include "cfg.hpp"

#include <cstddef>
#include <sstream>
#include <string>

namespace {
const int PORT_MIN = 1;
const int PORT_MAX = 65535;
// Per-client send backlog: how many unsent bytes we buffer before dropping a
// client that won't drain. Operator-tunable; defaults to 1 MiB.
const std::size_t DEFAULT_MAX_SEND_QUEUE = 1024 * 1024;
} // namespace

int Config::port() const { return _port; }

const std::string &Config::ip() const { return _ip; }

std::size_t Config::maxSendQueue() const { return _maxSendQueue; }

Config::Config(const std::string &path) {
    int port;
    std::string ip;
    std::size_t maxSendQueue;

    try {
        cfg::Statement root = cfg::parseFile(path);
        // pointeur pour éviter le faux positif -Wdangling-reference de getBlock
        const cfg::Statement *server = &cfg::getBlock(root, "server");
        port = cfg::getInt(*server, "port");
        ip = cfg::getString(*server, "ip");
        maxSendQueue =
            cfg::getSize(*server, "max_send_queue", DEFAULT_MAX_SEND_QUEUE);
    } catch (const cfg::Error &e) {
        throw Config::Error(e.what());
    }

    if (port < PORT_MIN || port > PORT_MAX) {
        std::ostringstream oss;
        oss << "port out of range (" << PORT_MIN << "-" << PORT_MAX << "): got "
            << port;
        throw Config::Error(oss.str());
    }
    if (maxSendQueue == 0) {
        throw Config::Error("max_send_queue must be greater than 0");
    }
    _port = port;
    _ip = ip;
    _maxSendQueue = maxSendQueue;
}
