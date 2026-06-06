#include "Config.hpp"
#include "cfg.hpp"

#include <sstream>

namespace {
    const int PORT_MIN = 1;
    const int PORT_MAX = 65535;
}

int Config::port() const {
    return _port;
}

Config::Config(const std::string& path) {
    int port;

    try {
        cfg::Statement root = cfg::parseFile(path);
        // pointeur pour éviter le faux positif -Wdangling-reference de getBlock
        const cfg::Statement* server = &cfg::getBlock(root, "server");
        port = cfg::getInt(*server, "port");
    }
    catch (const cfg::Error& e) {
        throw Config::Error(e.what());
    }

    if (port < PORT_MIN || port > PORT_MAX) {
        std::ostringstream oss;
        oss << "port out of range (" << PORT_MIN << "-" << PORT_MAX << "): got " << port;
        throw Config::Error(oss.str());
    }
    _port = port;
}