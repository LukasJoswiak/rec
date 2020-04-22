#include <chrono>
#include <deque>
#include <iostream>
#include <thread>

#include "client/tcp_client.hpp"
#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// From https://stackoverflow.com/a/12468109/986991
std::string RandomString(size_t length) {
  auto randchar = []() -> char {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = sizeof(charset) - 1;
    return charset[rand() % max_index];
  };

  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

int main(int argc, char** argv) {
  spdlog::set_level(spdlog::level::trace);
  spdlog::stdout_color_mt("client");
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e.%f] [%n] [%^%l%$] %v");

  std::string client_name = argv[1];
  std::string server_port = argv[2];
  int num_clients = std::stoi(argv[3]);
  int requests_per_client = std::stoi(argv[4]);
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

  for (int i = 1; i <= num_clients; ++i) {
    for (int j = 1; j <= requests_per_client; ++j) {
      std::string client = "client" + std::to_string(i);
      Command c;
      c.set_client(client);
      c.set_sequence_number(j);
      c.set_key("foo" + std::to_string(j));
      c.set_value(RandomString(10000050));
      c.set_operation(Command_Operation_PUT);
      workload[client].push_back(c);
    }
  }

  TcpClient client(workload, num_clients * requests_per_client);
  client.Start(client_name, std::stoi(server_port));

  return 0;
}
