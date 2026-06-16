#include "util/Buffer.hpp"

namespace util {

Buffer::Buffer() : _readPos(0) {}

void Buffer::append(const char *data, std::size_t len) {
    _data.append(data, len);
}

const char *Buffer::peek() const { return _data.data() + _readPos; }

std::size_t Buffer::size() const { return _data.size() - _readPos; }

bool Buffer::empty() const { return _readPos == _data.size(); }

void Buffer::consume(std::size_t n) {
    if (n > size()) {
        n = size();
    }
    _readPos += n;

    if (_readPos == _data.size()) {
        // Everything consumed: cheapest reset (keeps the string's capacity).
        _data.clear();
        _readPos = 0;
    } else if (_readPos > _data.size() / 2) {
        // Dead prefix now exceeds live data: compact. Amortized O(1) since we
        // only move the live bytes after having consumed at least as many.
        _data.erase(0, _readPos);
        _readPos = 0;
    }
}

} // namespace util
