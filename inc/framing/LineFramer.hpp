#ifndef FRAMING_LINEFRAMER_HPP
#define FRAMING_LINEFRAMER_HPP

#include "util/Buffer.hpp"

#include <cstddef>
#include <string>

namespace framing {

// Extracts CR/LF-terminated lines from a byte buffer. A line longer than maxLen
// is truncated to its first maxLen bytes (the connection is NOT dropped), which
// also bounds the buffer when a peer sends an over-long unterminated line. Holds
// minimal state so a line spanning several buffer fills is still truncated once.
class LineFramer {
  public:
    explicit LineFramer(std::size_t maxLen);

    // Pull the next complete line out of `in` (consuming it). Returns true and
    // sets `out` (without the trailing CR/LF) when a line is available.
    bool nextLine(util::Buffer &in, std::string &out);

  private:
    std::size_t _maxLen;
    bool _discarding;     // dropping the tail of an over-long line until '\n'
    std::string _captured; // truncated content held while discarding
};

} // namespace framing

#endif
