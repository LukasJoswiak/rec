// Copyright 2019 Lukas Joswiak

#include <array>
#include <iostream>

#include <boost/asio.hpp>

int main(int argc, char** argv) {
  try {
    boost::asio::io_context io_context;

    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(
        argv[1], "12345");

    boost::asio::ip::tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    for (;;) {
      std::array<char, 128> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof) {
        // Connection closed by peer.
        break;
      } else if (error) {
        throw boost::system::system_error(error);
      }

      std::cout.write(buf.data(), len);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
