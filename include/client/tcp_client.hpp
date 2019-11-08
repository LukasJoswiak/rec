// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_CLIENT_TCP_CLIENT_HPP_
#define INCLUDE_CLIENT_TCP_CLIENT_HPP_

#include <boost/asio.hpp>

class TcpClient {
 public:
  explicit TcpClient(boost::asio::io_context& io_context);

  // Starts the client and attempts to open a connection using the given
  // endpoints.
  void Start(boost::asio::ip::tcp::resolver::results_type endpoints);

  // Closes the connection.
  void Stop();

 private:
  // Attempts to connect to the endpoint.
  void StartConnect(
      boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter);

  // Handles a connection attempt to the endpoint.
  void HandleConnect(
      const boost::system::error_code& error,
      boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter);

  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::resolver::results_type endpoints_;
};

#endif  // INCLUDE_CLIENT_TCP_CLIENT_HPP_
