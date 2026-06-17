#include "io/Clock.hpp"

#include <time.h>

long io::monotonicMs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<long>(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}
