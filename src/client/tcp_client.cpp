// Copyright 2019 Lukas Joswiak

#include "client/tcp_client.hpp"

TcpClient::TcpClient(boost::asio::io_context& io_context)
    : socket_(io_context) {}

void TcpClient::Start(boost::asio::ip::tcp::resolver::results_type endpoints) {
}

void TcpClient::StartConnect(
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter) {
}

void TcpClient::HandleConnect(
    boost::system::error_code& error,
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter) {
}

