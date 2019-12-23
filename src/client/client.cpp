// Copyright 2019 Lukas Joswiak

#include <array>
#include <iostream>

#include <boost/asio.hpp>

#include "client/tcp_client.hpp"

int main(int argc, char** argv) {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::resolver r(io_context);
  TcpClient client(io_context);

  client.Start(r.resolve("localhost", "1112"));

  io_context.run();

  return 0;
}
