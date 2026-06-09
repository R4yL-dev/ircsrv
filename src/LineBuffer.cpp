#include "LineBuffer.hpp"
#include <cstddef>
#include <string>

LineBuffer::LineBuffer(std::size_t maxLineLength)
    : _maxLineLength(maxLineLength), _lineLen(0) {}

void LineBuffer::append(const char *data, std::size_t len) {
    for (std::size_t i = 0; i < len; ++i) {
        char c = data[i];
        if (c == '\n') {
            _buf += c;
            _lineLen = 0;
        } else if (_lineLen < _maxLineLength) {
            _buf += c;
            ++_lineLen;
        }
    }
}

bool LineBuffer::getLine(std::string &out) {
    std::string::size_type pos = _buf.find('\n');

    if (pos == std::string::npos) {
        return false;
    }
    out = _buf.substr(0, pos);

    if (!out.empty() && out[out.size() - 1] == '\r') {
        out.erase(out.size() - 1);
    }

    _buf.erase(0, pos + 1);
    return true;
}