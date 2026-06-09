#ifndef ERROR_HPP
#define ERROR_HPP

#include <exception>
#include <string>

class FatalError : public std::exception {
  public:
    explicit FatalError(const std::string &msg);
    virtual ~FatalError() throw();
    virtual const char *what() const throw();

  private:
    std::string _msg;
};

#endif
