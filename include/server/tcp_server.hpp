#ifndef INCLUDE_SERVER_TCP_SERVER_HPP_
#define INCLUDE_SERVER_TCP_SERVER_HPP_

#include <memory>

#include "spdlog/spdlog.h"
#include "server/connection_manager.hpp"
#include "server/tcp_socket.hpp"

class TcpServer {
 public:
  explicit TcpServer(std::string&& name, uint16_t port);

  // Runs the server. Attempts to connect to other servers, then begins
  // listening for incoming connections.
  bool Run();

 private:
  // Converts the given endpoint into a sockaddr_storage struct. `ret_addr` and
  // `ret_addrlen` are return parameters which will be populated on success. 
  // Returns true on success.
  bool LookupName(std::string& host, unsigned short port,
      struct sockaddr_storage* ret_addr, std::size_t* ret_addrlen);

  // Creates a socket to the given address and connects to it. Sets newly
  // created file descriptor on `ret_fd` return parameter. Returns true on
  // success.
  bool Connect(const struct sockaddr_storage& addr, const std::size_t& addrlen,
      int* ret_fd);

  std::string name_;
  TcpSocket ts_;
  std::shared_ptr<ConnectionManager> connection_manager_;

  std::shared_ptr<spdlog::logger> logger_;
};

#endif  // INCLUDE_SERVER_TCP_SERVER_HPP_
