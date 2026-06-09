#ifndef IO_FDHANDLE_HPP
#define IO_FDHANDLE_HPP

namespace io {

class FdHandle {
  public:
    explicit FdHandle(int fd);
    ~FdHandle();

    int fd() const;
    int release();

  private:
    int _fd;

    FdHandle(const FdHandle &);
    FdHandle &operator=(const FdHandle &);
};

} // namespace io

#endif
