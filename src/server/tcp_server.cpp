#include "server/tcp_server.hpp"

#include "server/servers.hpp"
#include "server/tcp_connection.hpp"

TcpServer::TcpServer(std::string&& name, uint16_t port)
    : name_(name),
      ts_(port),
      connection_manager_(std::make_shared<ConnectionManager>(name)) {
  logger_ = spdlog::get("server");
}

bool TcpServer::Run() {
  int listen_fd;
  if (!ts_.Listen(AF_INET6, &listen_fd)) {
    logger_->error("failed to bind to listen socket");
    return false;
  }

  // Attempt to connect to other servers.
  for (int i = 0; i < kServers.size(); ++i) {
    auto server_name = std::get<0>(kServers.at(i));
    auto server_port = std::get<1>(kServers.at(i));
    if (server_name != name_) {
      logger_->info("attempting to open connection to {}", server_name);

      std::string host = "localhost";
      struct sockaddr_storage addr;
      std::size_t addrlen;
      if (!LookupName(host, server_port, &addr, &addrlen)) {
        continue;
      }

      int socket_fd;
      if (!Connect(addr, addrlen, &socket_fd)) {
        continue;
      }
      logger_->info("connected to {}", server_name);

      // Keep track of connection to other server.
      auto connection = std::make_shared<TcpConnection>(socket_fd,
          connection_manager_, server_name);

      // Start tracking the new connnection.
      connection_manager_->AddServerConnection(connection);

      // Start the connection.
      std::thread([&] () {
        connection->Start();
      }).detach();

      // Send a setup message to tell the remote server the name of this server.
      Message message;
      message.set_type(Message_MessageType_SETUP);
      message.set_from(name_);
      connection_manager_->Deliver(message, server_name);
    }
  }

  // Now, listen for incoming connections.
  logger_->debug("accepting connections...");
  while (1) {
    int peer_fd;
    if (!ts_.Accept(&peer_fd)) {
      break;
    }

    // TODO: Switch to a thread pool with a set number of threads.
    std::thread([&] () {
      logger_->trace("accepted connection");
      std::string endpoint = "";
      auto connection =
          std::make_shared<TcpConnection>(peer_fd, connection_manager_, endpoint);
      connection->Start();
    }).detach();
  }

  return true;
}

// TODO: This functionality is shared by the client and server -- factor it out.
bool TcpServer::LookupName(std::string& host, unsigned short port,
    struct sockaddr_storage* ret_addr, std::size_t* ret_addrlen) {
  int status;
  struct addrinfo hints;
  struct addrinfo* results;

  memset(&hints, 0, sizeof(hints));
  // IPv4 only for now.
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(host.c_str(), nullptr, &hints, &results)) != 0) {
    logger_->error("getaddrinfo error: {}", gai_strerror(status));
    return false;
  }
  
  // Use the first result from getaddrinfo.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in* v4_addr =
        reinterpret_cast<struct sockaddr_in*>(results->ai_addr);
    v4_addr->sin_port = htons(port);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6* v6_addr =
        reinterpret_cast<struct sockaddr_in6*>(results->ai_addr);
    v6_addr->sin6_port = htons(port);
  } else {
    logger_->error("getaddrinfo failed to provide address");
    freeaddrinfo(results);
    return false;
  }

  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_addrlen = results->ai_addrlen;

  freeaddrinfo(results);
  return true;
}

bool TcpServer::Connect(const struct sockaddr_storage& addr,
    const std::size_t& addrlen, int* ret_fd) {
  int socket_fd;
  if ((socket_fd = socket(addr.ss_family, SOCK_STREAM, 0)) == -1) {
    logger_->error("socket() error: {}", strerror(errno));
    return false;
  }

  // Disable Nagle's algorithm.
  int flag;
  int result = setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (char*) &flag,
                          sizeof(int));
  if (result == -1) {
    logger_->error("failed to enable TCP_NODELAY");
  }

  int res;
  if ((res = connect(socket_fd,
      reinterpret_cast<const sockaddr*>(&addr), addrlen)) == -1) {
    logger_->error("connect() error: {}", strerror(errno));
    return false;
  }

  *ret_fd = socket_fd;
  return true;
}
