#include "IrcServ.hpp"
#include "Config.hpp"

#include <iostream>

static void show_infos();

int main(int ac, char **av) {
    (void)ac;
    (void)av;
    show_infos();

    std::cout << "PARSING CONFIG...\n";
    try {
        Config config("server.conf");
        std::cout << " - port = " << config.port() << "\n";
    }
    catch (const Config::Error& e) {
        std::cerr << "config error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "RUNNING SERVER...\n";

	return 0;
}

void show_infos() {
    std::cout << "IRCServ\n";
    std::cout << " - Version: " << VERSION << "\n";
    std::cout << " - Build date: " << __DATE__ << "\n\n";
}
