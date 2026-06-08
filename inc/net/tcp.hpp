#ifndef NET_TCP_HPP
#define NET_TCP_HPP

#include <string>

namespace net {

int tcpListen(const std::string &host, int port);
int tcpAccept(int listenFd);

} // namespace net

#endif
