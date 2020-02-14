#ifndef INCLUDE_SERVER_TCP_CONNECTION_HPP_
#define INCLUDE_SERVER_TCP_CONNECTION_HPP_

#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include <boost/asio.hpp>

#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// Forward declare the ConnectionManager class to break the circular dependency.
class ConnectionManager;

// Represents a TCP connection with a host.
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  // Creates and returns a new TcpConnection object using the provided context.
  static std::shared_ptr<TcpConnection> Create(
      boost::asio::io_context& io_context, ConnectionManager& manager,
      std::string endpoint_name);

  // Disable copy constructor and assignment operator.
  TcpConnection(const TcpConnection& other) = delete;
  TcpConnection& operator=(const TcpConnection& other) = delete;

  // Shuts the connection down on destruction.
  ~TcpConnection();

  // Call this method when a new connection has been established, to allow
  // initial setup to be performed.
  void Start();

  // Queues the message to be sent.
  void Write(const Message& message);

  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  // Returns the name of the remote endpoint.
  std::string endpoint_name() {
    return endpoint_name_;
  }

  void set_endpoint_name(std::string endpoint_name) {
    endpoint_name_ = endpoint_name;
  }

 private:
  explicit TcpConnection(boost::asio::io_context& io_context,
                         ConnectionManager& manager, std::string endpoint_name);

  static const int kHeaderSize = 4;

  // Asynchronously reads kHeaderSize bytes to determine the size of the
  // message.
  void StartReadHeader();

  // Handler called when the message size has been read.
  void HandleReadHeader(const boost::system::error_code& error, std::size_t n);
  // Handler called when the message has been read.
  void HandleReadBody(const boost::system::error_code& error, std::size_t n);

  // Asynchronously writes the message at the front of the message queue to the
  // socket provided at instance creation.
  void Write();

  // Handler called after a message is written to the socket.
  void HandleWrite(const boost::system::error_code& error,
                   size_t bytes_transferred);

  boost::asio::ip::tcp::socket socket_;
  boost::asio::io_context::strand strand_;
  char inbound_header_[kHeaderSize];
  boost::asio::streambuf input_buffer_;
  ConnectionManager& manager_;
  std::string endpoint_name_;

  // Lock around queue access. The asynchronous write and callback handler will
  // be called on a separate thread from the start write function, so it is
  // necessary to lock to avoid concurrency bugs with the queue.
  std::mutex mutex_;
  // Queue for outbound messages.
  std::deque<std::string> out_queue_;

  std::shared_ptr<spdlog::logger> logger_;
};

#endif  // INCLUDE_SERVER_TCP_CONNECTION_HPP_
