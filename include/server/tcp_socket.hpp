#ifndef INCLUDE_SERVER_TCP_SOCKET_HPP_
#define INCLUDE_SERVER_TCP_SOCKET_HPP_

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "spdlog/spdlog.h"

// Wraps functionality to create a TCP listening socket.
class TcpSocket {
 public:
  explicit TcpSocket(uint16_t port);
  virtual ~TcpSocket();

  // Creates a listening socket and binds it to the port number passed at
  // construction. `ai_family` is a value representing whether to create a
  // listener for IPv4 (AF_INET), IPv6 (AF_INET6) or both (AF_UNSPEC) kinds of
  // address. `listen_fd` is a return parameter used to pass the created sockets
  // file descriptor back to the caller. Returns true on success.
  bool Listen(int ai_family, int* listen_fd);

  // Accepts an incoming connection, creating a file descriptor for it and
  // returning the file descriptor in the `accepted_fd` return parameter. Blocks
  // until a connection is made.
  bool Accept(int* accepted_fd);

 private:
  uint16_t port_;
  int listen_fd_;
  int socket_family_;

  std::shared_ptr<spdlog::logger> logger_;
};

#endif  // INCLUDE_SERVER_TCP_SOCKET_HPP_
