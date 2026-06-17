#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef>
#include <stdexcept>
#include <string>

class Config {
  public:
    class Error : public std::runtime_error {
      public:
        explicit Error(const std::string &msg) : std::runtime_error(msg) {}
    };

    explicit Config(const std::string &path);

    int port() const;
    const std::string &ip() const;
    std::size_t maxSendQueue() const;

  private:
    int _port;
    std::string _ip;
    std::size_t _maxSendQueue;
};

#endif
