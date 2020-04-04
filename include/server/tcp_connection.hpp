#ifndef INCLUDE_SERVER_TCP_CONNECTION_HPP_
#define INCLUDE_SERVER_TCP_CONNECTION_HPP_

#include <memory>

#include "process/shared_queue.hpp"
#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"

// Forward declare the ConnectionManager class to break the circular dependency.
class ConnectionManager;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  explicit TcpConnection(int fd, std::shared_ptr<ConnectionManager> manager,
      std::string& endpoint_name);
  ~TcpConnection();

  // Disable copy constructor and assignment operator.
  TcpConnection(const TcpConnection& other) = delete;
  TcpConnection& operator=(const TcpConnection& other) = delete;

  // Spawns threads to read and write data from the socket. Blocks until
  // the reader and writer thread returns. This function should be run on its
  // own thread.
  void Start();

  // Closes the socket connection.
  void Stop();

  void Write(const Message& message);

  std::string endpoint() {
    return endpoint_;
  }

  void set_endpoint(const std::string& endpoint) {
    endpoint_ = endpoint;
  }

 private:
  static const int kHeaderSize = 4;

  // Continuously reads messages from the file descriptor for this connection.
  // The first kHeaderSize bytes of each message must be the size of the
  // message. This function is blocking and should be run on its own thread.
  void Read();

  // Reads the given number of bytes into buf from the socket with the given
  // file descriptor. Returns true on success.
  bool Read(int fd, char* buf, std::size_t bytes);

  // Continuously writes messages from `out_queue_` to the socket associated
  // with this connection. This function blocks while waiting for `out_queue_`
  // to be populated with at least one message, and sholud be run on its own
  // thread.
  void Write();

  // Writes the given message to the socket with the given file descriptor.
  // Returns true on success.
  bool Write(int fd, const std::string& message);

  int fd_;

  // TODO: Use a circular buffer
  // Output queue for messages.
  process::common::SharedQueue<std::string> out_queue_;

  std::shared_ptr<ConnectionManager> manager_;
  std::string endpoint_;

  std::shared_ptr<spdlog::logger> logger_;
};

#endif  // INCLUDE_SERVER_TCP_CONNECTION_HPP_
