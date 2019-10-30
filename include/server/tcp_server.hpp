#ifndef TCP_SERVER_HPP_
#define TCP_SERVER_HPP_

#include <memory>

#include <boost/asio.hpp>

#include "server/tcp_connection.hpp"

class TcpServer {
 public:
  TcpServer(boost::asio::io_context& io_context);

 private:
  void StartAccept();

  void HandleAccept(std::shared_ptr<TcpConnection> new_connection,
                    const boost::system::error_code& error);

  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

#endif  // TCP_SERVER_HPP_
