#include "server/tcp_socket.hpp"

TcpSocket::TcpSocket(uint16_t port) : port_(port), listen_fd_(-1) {
  logger_ = spdlog::get("socket");
}

TcpSocket::~TcpSocket() {
  if (listen_fd_ != -1) {
    close(listen_fd_);
  }
  listen_fd_ = -1;
}

bool TcpSocket::Listen(int ai_family, int* listen_fd) {
  assert(ai_family == AF_INET || ai_family == AF_INET6 ||
      ai_family == AF_UNSPEC);

  int res;
  struct addrinfo hints;
  struct addrinfo* results;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = ai_family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (ai_family == AF_INET6) {
    hints.ai_flags |= AI_V4MAPPED;
  }
  hints.ai_protocol = IPPROTO_TCP;

  std::string port = std::to_string(port_);
  res = getaddrinfo(nullptr, port.c_str(), &hints, &results);
  if (res != 0) {
    logger_->error("getaddrinfo failed: {}", gai_strerror(res));
    return false;
  }

  struct addrinfo* rp;
  for (rp = results; rp != nullptr; rp = rp->ai_next) {
    listen_fd_ = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (listen_fd_ == -1) {
      continue;
    }

    // Disable Nagle's algorithm.
    int flag;
    if (setsockopt(listen_fd_, IPPROTO_TCP, TCP_NODELAY, (char*) &flag,
          sizeof(int)) != 0) {
      logger_->error("failed to enable TCP_NODELAY");
    }

    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag,
          sizeof(flag)) != 0) {
      logger_->error("failed to set socket as reusable");
    }

    if (bind(listen_fd_, rp->ai_addr, rp->ai_addrlen) == 0) {
      socket_family_ = rp->ai_family;
      break;
    }

    // Try again with next result.
    close(listen_fd_);
    listen_fd_ = -1;
  }

  if (rp == nullptr) {
    logger_->error("failed to find a valid address");
    freeaddrinfo(results);
    return false;
  }

  if (listen(listen_fd_, SOMAXCONN) != 0) {
    logger_->error("failed to listen on socket: {}", strerror(errno));
    freeaddrinfo(results);
    close(listen_fd_);
    listen_fd_ = -1;
    return false;
  }

  *listen_fd = listen_fd_;
  freeaddrinfo(results);
  return true;
}

bool TcpSocket::Accept(int* accepted_fd) {
  int peer_fd;
  struct sockaddr_in peer4;
  struct sockaddr_in6 peer6;
  struct sockaddr* peer;
  socklen_t peerlen;

  if (socket_family_ == AF_INET) {
    peer = (struct sockaddr*) &peer4;
    peerlen = sizeof(peer4);
  } else {
    peer = (struct sockaddr*) &peer6;
    peerlen = sizeof(peer6);
  }

  while (1) {
    peer_fd = accept(listen_fd_, peer, &peerlen);
    if (peer_fd == -1) {
      if (errno == EINTR) {
        continue;
      }
      logger_->error("accept failed: {}", strerror(errno));
      return false;
    }
    break;
  }

  int flag;
  if (setsockopt(peer_fd, SOL_SOCKET, SO_REUSEADDR, &flag,
        sizeof(flag)) != 0) {
    logger_->error("failed to set socket as reusable");
  }

  *accepted_fd = peer_fd;
  return true;
}
