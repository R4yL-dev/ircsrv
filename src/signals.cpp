#include "signals.hpp"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <stdexcept>

namespace {
volatile sig_atomic_t g_stop = 0;
void handle(int) { g_stop = 1; }
} // namespace

void signals::setup() {
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle;
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        throw std::runtime_error(std::strerror(errno));
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        throw std::runtime_error(std::strerror(errno));
    }
}

bool signals::stopRequested() { return g_stop != 0; }