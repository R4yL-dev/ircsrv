#ifndef SIGNALS_HPP
#define SIGNALS_HPP

#include <csignal>

namespace signals {

sigset_t shutdownMask();

} // namespace signals

#endif
