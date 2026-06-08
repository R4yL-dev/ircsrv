#include "Config.hpp"
#include "cfg.hpp"

#include <sstream>
#include <string>

namespace {
const int PORT_MIN = 1;
const int PORT_MAX = 65535;
} // namespace

int Config::port() const { return _port; }

const std::string &Config::ip() const { return _ip; }

Config::Config(const std::string &path) {
    int port;
    std::string ip;

    try {
        cfg::Statement root = cfg::parseFile(path);
        // pointeur pour éviter le faux positif -Wdangling-reference de getBlock
        const cfg::Statement *server = &cfg::getBlock(root, "server");
        port = cfg::getInt(*server, "port");
        ip = cfg::getString(*server, "ip");
    } catch (const cfg::Error &e) {
        throw Config::Error(e.what());
    }

    if (port < PORT_MIN || port > PORT_MAX) {
        std::ostringstream oss;
        oss << "port out of range (" << PORT_MIN << "-" << PORT_MAX << "): got "
            << port;
        throw Config::Error(oss.str());
    }
    _port = port;
    _ip = ip;
}
