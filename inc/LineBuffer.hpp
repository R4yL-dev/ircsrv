#ifndef LINEBUFFER_HPP
#define LINEBUFFER_HPP

#include <cstddef>
#include <string>

class LineBuffer {
  public:
    explicit LineBuffer(std::size_t maxLineLength);

    bool append(const char *data, std::size_t len);
    bool getLine(std::string &out);

  private:
    std::string _buf;
    std::size_t _maxLineLength;
};

#endif
