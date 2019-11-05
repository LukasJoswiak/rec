// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_TCP_CONNECTION_HPP_
#define INCLUDE_SERVER_TCP_CONNECTION_HPP_

#include <memory>
#include <string>

#include <boost/asio.hpp>

// Forward declare the Handler class to break the circular dependency.
class Handler;

// Represents a TCP connection with a host.
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  // Creates and returns a new TcpConnection object using the provided context.
  static std::shared_ptr<TcpConnection> Create(
      boost::asio::io_context& io_context, Handler& handler);

  // Disable copy constructor and assignment operator.
  TcpConnection(const TcpConnection& other) = delete;
  TcpConnection& operator=(const TcpConnection& other) = delete;

  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  // Returns a string representation of the local endpoint of the connection.
  std::string LocalEndpoint();

  // Returns a string representation of the remote endpoint of the connection.
  std::string RemoteEndpoint();

  // Call this method when a new connection has been established, to allow
  // initial setup to be performed.
  void Start();

  // Asynchronously writes the message to the socket provided at instance
  // creation.
  void StartWrite(const std::string& message);

 private:
  explicit TcpConnection(boost::asio::io_context& io_context, Handler& handler);

  // Asynchronously reads until the end of line character is found.
  void StartRead();

  // Handler called after a message has been read.
  void HandleRead(const boost::system::error_code& error, std::size_t n);

  // Handler called after a message is written to the socket.
  void HandleWrite(const boost::system::error_code& ec,
                   size_t bytes_transferred);

  boost::asio::ip::tcp::socket socket_;
  std::string input_buffer_;

  Handler& handler_;
};

#endif  // INCLUDE_SERVER_TCP_CONNECTION_HPP_
