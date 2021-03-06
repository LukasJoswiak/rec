#include <chrono>
#include <functional>
#include <thread>

#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "server/tcp_server.hpp"

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  try {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e.%f] [%^%l%$] [%t] [%n] %v");
    spdlog::set_level(spdlog::level::trace);
    spdlog::stdout_color_mt("echo");
    spdlog::stdout_color_mt("replica");
    spdlog::stdout_color_mt("acceptor");
    spdlog::stdout_color_mt("leader");
    spdlog::stdout_color_mt("scout");
    spdlog::stdout_color_mt("commander");
    spdlog::stdout_color_mt("socket");
    spdlog::stdout_color_mt("server");
    spdlog::stdout_color_mt("manager");
    spdlog::stdout_color_mt("connection");

    int port = std::stoi(argv[2]);

    // Create and run server.
    // TcpServer server(argv[1], port);
    TcpServer server(argv[1], port);
    server.Run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
