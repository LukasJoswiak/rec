#include <chrono>
#include <deque>
#include <iostream>
#include <thread>

#include "client/tcp_client.hpp"
#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace {
const int kValueLength = 500;
}

std::string GenerateValue() {
  // TODO: Generate random sequences
  return std::string(kValueLength, 'a');
}

int main(int argc, char** argv) {
  spdlog::set_level(spdlog::level::trace);
  spdlog::stdout_color_mt("client");
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e.%f] [%n] [%^%l%$] %v");

  std::string client_name = argv[1];
  std::string server_port = argv[2];
  // Create a workload for each client.
  std::unordered_map<std::string, std::deque<Command>> workload;

  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  /*
  {
    Command c;
    c.set_client("client1");
    c.set_sequence_number(1);
    c.set_key("foo");
    c.set_value("barbarbarbar");
    c.set_operation(Command_Operation_PUT);
    workload["client1"].push_back(c);
  }

  {
    Command c;
    c.set_client("client2");
    c.set_sequence_number(1);
    c.set_key("bar");
    c.set_value("bazbazbazbazbazbaz");
    c.set_operation(Command_Operation_PUT);
    workload["client2"].push_back(c);
  }

  {
    Command c;
    c.set_client("client1");
    c.set_sequence_number(2);
    c.set_key("foo");
    c.set_operation(Command_Operation_GET);
    workload["client1"].push_back(c);
  }

  {
    Command c;
    c.set_client("client2");
    c.set_sequence_number(2);
    c.set_key("bar");
    c.set_operation(Command_Operation_GET);
    workload["client2"].push_back(c);
  }
  */

  int num_clients = 100;
  int requests_per_client = 100;
  for (int i = 1; i <= num_clients; ++i) {
    for (int j = 1; j <= requests_per_client; ++j) {
      std::string client = "client" + std::to_string(i);
      Command c;
      c.set_client(client);
      c.set_sequence_number(j);
      c.set_key("foo" + std::to_string(j));
      c.set_value("barbarbarbar");
      c.set_operation(Command_Operation_PUT);
      workload[client].push_back(c);
    }
  }

  TcpClient client(workload);
  client.Start("localhost", std::stoi(server_port));

  return 0;
}
