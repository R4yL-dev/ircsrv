#include "framing/LineFramer.hpp"

#include <cstring>

static void stripTrailingCR(std::string &s);
// Index of the first '\n' in the readable bytes, or in.size() if there is none.
static std::size_t findNewline(const util::Buffer &in);

framing::LineFramer::LineFramer(std::size_t maxLen)
    : _maxLen(maxLen), _discarding(false) {}

bool framing::LineFramer::nextLine(util::Buffer &in, std::string &out) {
    if (_discarding) {
        std::size_t p = findNewline(in);
        if (p == in.size()) {
            in.consume(in.size()); // drop the over-long tail, keep waiting
            return false;
        }
        in.consume(p + 1); // drop tail + '\n'
        out = _captured;
        stripTrailingCR(out);
        _captured.clear();
        _discarding = false;
        return true;
    }

    std::size_t p = findNewline(in);
    if (p == in.size()) {
        if (in.size() > _maxLen) {
            // Over-long line whose terminator has not arrived yet: capture the
            // truncated content now and discard the rest until the next '\n'.
            _captured.assign(in.peek(), _maxLen);
            in.consume(in.size());
            _discarding = true;
        }
        return false;
    }

    if (p > _maxLen) {
        out.assign(in.peek(), _maxLen); // truncate over-long line
    } else {
        out.assign(in.peek(), p);
    }
    in.consume(p + 1);
    stripTrailingCR(out);
    return true;
}

void stripTrailingCR(std::string &s) {
    if (!s.empty() && s[s.size() - 1] == '\r') {
        s.erase(s.size() - 1);
    }
}

std::size_t findNewline(const util::Buffer &in) {
    const char *base = in.peek();
    const void *nl = std::memchr(base, '\n', in.size());
    if (nl == 0) {
        return in.size();
    }
    return static_cast<std::size_t>(static_cast<const char *>(nl) - base);
}
