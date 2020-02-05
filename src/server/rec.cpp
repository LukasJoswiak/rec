#include <chrono>
#include <functional>
#include <thread>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "server/tcp_server.hpp"

int main(int argc, char** argv) {
  try {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] [%n] %v");
    spdlog::set_level(spdlog::level::trace);
    spdlog::stdout_color_mt("echo");
    spdlog::stdout_color_mt("replica");
    spdlog::stdout_color_mt("acceptor");
    spdlog::stdout_color_mt("leader");
    spdlog::stdout_color_mt("scout");
    spdlog::stdout_color_mt("commander");

    int port = std::stoi(argv[2]);
    // TODO: remove. This is a bit of a hack so I can use the synchronize-panes
    // tmux option and avoid having to fix some sort of issue when starting
    // all three replicas at the same time.
    // std::this_thread::sleep_for(std::chrono::milliseconds((port - 1111) * 50));

    boost::asio::io_context io_context;
    TcpServer server(io_context, argv[1], port);

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
