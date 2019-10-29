#include <functional>
#include <iostream>
#include <memory>

#include <boost/asio.hpp>

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  static std::shared_ptr<TcpConnection> Create(
      boost::asio::io_context& io_context) {
    return std::shared_ptr<TcpConnection>(new TcpConnection(io_context));
  }

  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  void Start() {
    message_ = "Hello";
    boost::asio::async_write(socket_, boost::asio::buffer(message_),
                             std::bind(&TcpConnection::HandleWrite,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
  }

 private:
  TcpConnection(boost::asio::io_context& io_context) : socket_(io_context) {}

  void HandleWrite(const boost::system::error_code& ec,
                   size_t bytes_transferred) {
    std::cout << "Wrote " << bytes_transferred << " bytes" << std::endl;
  }

  boost::asio::ip::tcp::socket socket_;
  std::string message_;
};

class TcpServer {
 public:
  TcpServer(boost::asio::io_context& io_context)
      : io_context_(io_context),
        acceptor_(io_context, boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(), 12345)) {
    StartAccept();
  }

 private:
  void StartAccept() {
    std::shared_ptr<TcpConnection> new_connection = TcpConnection::Create(
        io_context_);

    acceptor_.async_accept(new_connection->socket(),
                           std::bind(&TcpServer::HandleAccept, this,
                                     new_connection, std::placeholders::_1));
  }

  void HandleAccept(std::shared_ptr<TcpConnection> new_connection,
                    const boost::system::error_code& error) {
    if (!error) {
      new_connection->Start();
    }

    StartAccept();
  }

  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

int main() {
  try {
    boost::asio::io_context io_context;
    TcpServer server(io_context);

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
