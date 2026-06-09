#include "Config.hpp"
#include "Error.hpp"
#include "Server.hpp"
#include "io/Epoll.hpp"
#include "net/Socket.hpp"

#include <exception>
#include <iostream>

static void show_welcome_banner();

int main() {
    show_welcome_banner();

    try {
        Config config("server.conf");

        Server srv(config);
        std::cout << "Starting server on: " << config.ip() << ":"
                  << config.port() << "\n";
        srv.run();
    } catch (const Config::Error &e) {
        std::cerr << "config error: " << e.what() << "\n";
        return 1;
    } catch (const net::Socket::Error &e) {
        std::cerr << "socket error: " << e.what() << "\n";
        return 2;
    } catch (const io::Epoll::Error &e) {
        std::cerr << "epoll error: " << e.what() << "\n";
        return 3;
    } catch (const FatalError &e) {
        std::cerr << "fatal error: " << e.what() << "\n";
        return 4;
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 5;
    }

    return 0;
}

void show_welcome_banner() { std::cout << "Welcome to IRCServ !\n\n"; }
