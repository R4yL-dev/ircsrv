#include "Error.hpp"
#include <string>

FatalError::FatalError(const std::string &msg) : _msg(msg) {}

FatalError::~FatalError() throw() {}

const char *FatalError::what() const throw() { return _msg.c_str(); }