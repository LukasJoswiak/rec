#ifndef INCLUDE_SERVER_TCP_CONNECTION_HPP_
#define INCLUDE_SERVER_TCP_CONNECTION_HPP_

#include <memory>
#include <string>

#include <boost/asio.hpp>

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

  // Asynchronously writes the message to the socket provided at instance
  // creation.
  void StartWrite(const std::string& message);

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

  // Asynchronously reads until the end of line character is found.
  void StartRead();

  // Handler called after a message has been read.
  void HandleRead(const boost::system::error_code& error, std::size_t n);

  // Handler called after a message is written to the socket.
  void HandleWrite(const boost::system::error_code& error,
                   size_t bytes_transferred);

  boost::asio::ip::tcp::socket socket_;
  boost::asio::streambuf input_buffer_;
  ConnectionManager& manager_;
  std::string endpoint_name_;
};

#endif  // INCLUDE_SERVER_TCP_CONNECTION_HPP_
