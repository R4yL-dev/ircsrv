#ifndef IO_CLOCK_HPP
#define IO_CLOCK_HPP

namespace io {

// Milliseconds from a monotonic clock (CLOCK_MONOTONIC): never goes backwards and
// is unaffected by wall-clock jumps (NTP, DST), so it is the right source for
// measuring elapsed time and timeouts. The epoch is arbitrary -- only differences
// between two readings are meaningful.
long monotonicMs();

} // namespace io

#endif
