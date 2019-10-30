#ifndef TCP_CONNECTION_HPP_
#define TCP_CONNECTION_HPP_

#include <memory>
#include <string>

#include <boost/asio.hpp>

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  static std::shared_ptr<TcpConnection> Create(
      boost::asio::io_context& io_context);

  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  void Start();

 private:
  TcpConnection(boost::asio::io_context& io_context);

  void HandleWrite(const boost::system::error_code& ec,
                   size_t bytes_transferred);

  boost::asio::ip::tcp::socket socket_;
  std::string message_;
};

#endif  // TCP_CONNECTION_HPP_
