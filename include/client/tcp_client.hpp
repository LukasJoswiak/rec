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
  void StartWrite();

 private:
  static const int kHeaderSize = 4;

  // Attempts to connect to the endpoint.
  void StartConnect(
      boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter);

  // Handles a connection attempt to the endpoint.
  void HandleConnect(
      const boost::system::error_code& error,
      boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter);

  // Asynchronously reads kHeaderSize bytes to determine the size of the
  // message.
  void StartReadHeader();

  void HandleReadHeader(const boost::system::error_code& error, std::size_t n);
  void HandleReadBody(const boost::system::error_code& error, std::size_t n);

  // Handler called after a message is written to the socket.
  void HandleWrite(const boost::system::error_code& error);

  // Pulls the next request from the workload for the given client, and sends
  // the request to the server.
  void SendNextRequest(const std::string& client);

  // Creates a new serialized request message from the item at the beginning of
  // the workload for the given client, and removes the request from the
  // workload. The serialized message consists of a four-byte header followed
  // by the serialized message. Returns std::nullopt if no message is available
  // for the client.
  std::optional<std::string> GetNextMessage(const std::string& client);

  boost::asio::ip::tcp::socket socket_;
  boost::asio::io_context::strand strand_;
  boost::asio::ip::tcp::resolver::results_type endpoints_;
  char inbound_header_[kHeaderSize];
  boost::asio::streambuf input_buffer_;

  // Contains pairs of <serialized message, client id>.
  std::deque<std::pair<std::string, std::string>> out_queue_;

  std::string name_;
  // Map of client address -> time point when the most recent request was sent.
  std::unordered_map<std::string,
      std::chrono::time_point<std::chrono::steady_clock>> send_time_;
  // Map of client address -> deque of commands the client wants to run.
  std::unordered_map<std::string, std::deque<Command>> workload_;
  // Time the client was started.
  std::chrono::time_point<std::chrono::steady_clock> start_time_;

  std::shared_ptr<spdlog::logger> logger_;
};

#endif  // INCLUDE_CLIENT_TCP_CLIENT_HPP_
