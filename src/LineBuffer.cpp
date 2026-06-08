#include "LineBuffer.hpp"
#include <cstddef>
#include <string>

LineBuffer::LineBuffer(std::size_t maxLineLength)
    : _maxLineLength(maxLineLength) {}

bool LineBuffer::append(const char *data, std::size_t len) {
    _buf.append(data, len);

    if (_buf.find('\n') == std::string::npos && _buf.size() > _maxLineLength) {
        return false;
    }
    return true;
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