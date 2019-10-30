#include "server/tcp_server.hpp"

TcpServer::TcpServer(boost::asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, boost::asio::ip::tcp::endpoint(
          boost::asio::ip::tcp::v4(), 12345)) {
  StartAccept();
}

void TcpServer::StartAccept() {
  std::shared_ptr<TcpConnection> new_connection = TcpConnection::Create(
      io_context_);

  acceptor_.async_accept(new_connection->socket(),
                         std::bind(&TcpServer::HandleAccept, this,
                                   new_connection, std::placeholders::_1));
}

void TcpServer::HandleAccept(std::shared_ptr<TcpConnection> new_connection,
                  const boost::system::error_code& error) {
  if (!error) {
    new_connection->Start();
  }

  StartAccept();
}
