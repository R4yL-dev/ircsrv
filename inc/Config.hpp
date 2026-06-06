#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <stdexcept>

class Config {
public:
    class Error : public std::runtime_error {
    public:
        explicit Error(const std::string& msg) : std::runtime_error(msg) {}
    };

    explicit Config(const std::string& path);

    int port() const;

private:
    int _port;
};

#endif
