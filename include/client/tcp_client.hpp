#ifndef INCLUDE_CLIENT_TCP_CLIENT_HPP_
#define INCLUDE_CLIENT_TCP_CLIENT_HPP_

#include <chrono>
#include <deque>
#include <string>
#include <unordered_map>

#include <boost/asio.hpp>

#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"

class TcpClient {
 public:
  explicit TcpClient(boost::asio::io_context& io_context, std::string& name,
      std::unordered_map<std::string, std::deque<Command>>& workload);

  // Starts the client and attempts to open a connection using the given
  // endpoints.
  void Start(boost::asio::ip::tcp::resolver::results_type endpoints);

  // Closes the connection.
  void Stop();

  // Starts an asynchronous write of message on the socket.
  void StartWrite(boost::asio::streambuf& stream_buffer);

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

  // Pulls the next request from the workload for the given client, and sends
  // the request to the server.
  void SendNextRequest(const std::string& client);

  // Creates a new request message from the item at the beginning of the
  // workload for the given client, and removes the request from the workload.
  // Writes the message into the provided streambuf.
  void GetNextMessage(boost::asio::streambuf& stream_buffer,
      const std::string& client);

  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::resolver::results_type endpoints_;
  boost::asio::streambuf input_buffer_;

  std::string name_;
  // Map of client address -> time point when the most recent request was sent.
  std::unordered_map<std::string,
      std::chrono::time_point<std::chrono::steady_clock>> send_time_;
  // Map of client address -> deque of commands the client wants to run.
  std::unordered_map<std::string, std::deque<Command>> workload_;

  std::shared_ptr<spdlog::logger> logger_;
};

#endif  // INCLUDE_CLIENT_TCP_CLIENT_HPP_
