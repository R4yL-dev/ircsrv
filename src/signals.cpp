#include "signals.hpp"

#include <csignal>

sigset_t signals::shutdownMask() {
    sigset_t m;
    sigemptyset(&m);
    sigaddset(&m, SIGINT);
    sigaddset(&m, SIGTERM);

    return m;
}
