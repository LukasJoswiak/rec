// Copyright 2019 Lukas Joswiak

#include <functional>
#include <iostream>

#include "server/tcp_server.hpp"

int main(int argc, char** argv) {
  try {
    boost::asio::io_context io_context;
    TcpServer server(io_context, std::stoi(argv[1]));

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
