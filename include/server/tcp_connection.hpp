// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_TCP_CONNECTION_HPP_
#define INCLUDE_SERVER_TCP_CONNECTION_HPP_

#include <memory>
#include <string>

#include <boost/asio.hpp>

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  // Creates and returns a new TcpConnection object using the provided context.
  static std::shared_ptr<TcpConnection> Create(
      boost::asio::io_context& io_context);

  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  // Call this method when a new connection has been established, to allow
  // initial setup to be performed.
  void Start();

  // Delivers the message on the socket provided at instance creation.
  void Write(const std::string& message);

 private:
  explicit TcpConnection(boost::asio::io_context& io_context);

  // Handler called after a message is written to the socket.
  void HandleWrite(const boost::system::error_code& ec,
                   size_t bytes_transferred);

  boost::asio::ip::tcp::socket socket_;
};

#endif  // INCLUDE_SERVER_TCP_CONNECTION_HPP_
