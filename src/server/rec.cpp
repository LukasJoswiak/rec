// Copyright 2019 Lukas Joswiak

#include <functional>
#include <iostream>
#include <thread>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "server/tcp_server.hpp"

int main(int argc, char** argv) {
  try {
    spdlog::set_level(spdlog::level::trace);

    spdlog::stdout_color_mt("replica");
    spdlog::stdout_color_mt("acceptor");
    spdlog::stdout_color_mt("leader");
    spdlog::stdout_color_mt("scout");
    spdlog::stdout_color_mt("commander");

    boost::asio::io_context io_context;
    TcpServer server(io_context, argv[1], std::stoi(argv[2]));

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
