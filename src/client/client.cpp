// Copyright 2019 Lukas Joswiak

#include <chrono>
#include <deque>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>

#include "client/tcp_client.hpp"
#include "proto/messages.pb.h"

int main(int argc, char** argv) {
  std::string client_name = argv[1];
  // Create a workload.
  std::deque<Command> workload;

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  Command c;
  c.set_client(client_name);
  c.set_sequence_number(1);
  c.set_key("foo");
  c.set_value("barbarbarbar");
  c.set_operation(Command_Operation_PUT);
  workload.push_back(c);

  c.set_client(client_name);
  c.set_sequence_number(2);
  c.set_key("bar");
  c.set_value("baz");
  c.set_operation(Command_Operation_PUT);
  workload.push_back(c);

  c = Command();
  c.set_client(client_name);
  c.set_sequence_number(3);
  c.set_key("foo");
  c.set_operation(Command_Operation_GET);
  workload.push_back(c);

  c = Command();
  c.set_client(client_name);
  c.set_sequence_number(4);
  c.set_key("bar");
  c.set_operation(Command_Operation_GET);
  workload.push_back(c);

  boost::asio::io_context io_context;
  boost::asio::ip::tcp::resolver r(io_context);
  TcpClient client(io_context, client_name, workload);

  client.Start(r.resolve("localhost", "1113"));

  io_context.run();

  return 0;
}
