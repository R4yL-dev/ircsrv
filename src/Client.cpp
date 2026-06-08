#include "Client.hpp"

Client::Client(int fd) : _socket(fd) {}

int Client::fd() const { return _socket.fd(); }