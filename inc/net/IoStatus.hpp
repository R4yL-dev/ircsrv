#ifndef NET_IOSTATUS_HPP
#define NET_IOSTATUS_HPP

namespace net {

// Outcome of a non-blocking I/O step.
//   Ok         - made progress (read >= 1 byte, or output buffered/flushed)
//   WouldBlock - nothing available right now (EAGAIN); produced by the read
//                side only (fillInput / tcpAccept), never by the write side
//   Closed     - peer is gone, or an unrecoverable connection-level condition
// Genuine bugs (EBADF, EINVAL, ...) are reported as net::Socket::Error instead.
enum IoStatus { Ok, WouldBlock, Closed };

} // namespace net

#endif
