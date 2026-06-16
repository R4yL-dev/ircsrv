#ifndef UTIL_BUFFER_HPP
#define UTIL_BUFFER_HPP

#include <cstddef>
#include <string>

namespace util {

// A growable byte FIFO: append at the back, consume from the front.
// A read offset makes consume() amortized O(1) (no per-read shift); storage is
// compacted lazily so memory stays bounded to ~2x the live (unconsumed) data.
class Buffer {
  public:
    Buffer();

    void append(const char *data, std::size_t len);

    const char *peek() const; // start of the readable bytes (valid for size())
    std::size_t size() const; // number of readable bytes
    bool empty() const;

    void consume(std::size_t n); // drop n bytes from the front

  private:
    std::string _data;
    std::size_t _readPos;
};

} // namespace util

#endif
