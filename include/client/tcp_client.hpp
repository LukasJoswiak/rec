#ifndef INCLUDE_CLIENT_TCP_CLIENT_HPP_
#define INCLUDE_CLIENT_TCP_CLIENT_HPP_

#include <deque>
#include <optional>
#include <string>

#include <boost/asio.hpp>

#include "proto/messages.pb.h"

class TcpClient {
 public:
  explicit TcpClient(boost::asio::io_context& io_context, std::string& name,
                     std::deque<Command>& workload);

  // Starts the client and attempts to open a connection using the given
  // endpoints.
  void Start(boost::asio::ip::tcp::resolver::results_type endpoints);

  // Closes the connection.
  void Stop();

  // Starts an asynchronous write of message on the socket.
  void StartWrite(const std::string& message);

 private:
  // Attempts to connect to the endpoint.
  void StartConnect(
      boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter);

  // Handles a connection attempt to the endpoint.
  void HandleConnect(
      const boost::system::error_code& error,
      boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter);

  // Starts an asynchronous read operation to read from the socket until a
  // newline character is read.
  void StartRead();

  // Handler called after a message is read from the socket.
  void HandleRead(const boost::system::error_code& error,
                  std::size_t bytes_transferred);

  // Handler called after a message is written to the socket.
  void HandleWrite(const boost::system::error_code& error);

  // Creates a new request message from the item at the beginning of the
  // workload and returns an optional containing the serialized string
  // representation. Removes the request from the workload. If the workload is
  // empty, the returned optional will not contain a value.
  std::optional<std::string> GetNextMessage();

  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::resolver::results_type endpoints_;
  boost::asio::streambuf input_buffer_;

  std::string name_;
  std::deque<Command> workload_;
};

#endif  // INCLUDE_CLIENT_TCP_CLIENT_HPP_
