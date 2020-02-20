#include <chrono>
#include <deque>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>

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

  std::string client_name = argv[1];
  std::string server_port = argv[2];
  // Create a workload for each client.
  std::unordered_map<std::string, std::deque<Command>> workload;

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

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

  for (int i = 1; i <= 10000; ++i) {
    {
      Command c;
      c.set_client("client1");
      c.set_sequence_number(i);
      c.set_key("foo" + std::to_string(i));
      c.set_value("barbarbarbar");
      c.set_operation(Command_Operation_PUT);
      workload["client1"].push_back(c);
    }

    {
      Command c;
      c.set_client("client2");
      c.set_sequence_number(i);
      c.set_key("bar" + std::to_string(i));
      c.set_value("bazbazbazbazbazbaz");
      c.set_operation(Command_Operation_PUT);
      workload["client2"].push_back(c);
    }

    {
      Command c;
      c.set_client("client3");
      c.set_sequence_number(i);
      c.set_key("baz" + std::to_string(i));
      c.set_value("abcabcabcabc");
      c.set_operation(Command_Operation_PUT);
      workload["client3"].push_back(c);
    }

    {
      Command c;
      c.set_client("client4");
      c.set_sequence_number(i);
      c.set_key("raz" + std::to_string(i));
      c.set_value("zzzzzzzzzzzz");
      c.set_operation(Command_Operation_PUT);
      workload["client4"].push_back(c);
    }
  }

  boost::asio::io_context io_context;
  boost::asio::ip::tcp::resolver r(io_context);
  TcpClient client(io_context, client_name, workload);

  client.Start(r.resolve("localhost", server_port));

  io_context.run();

  return 0;
}
