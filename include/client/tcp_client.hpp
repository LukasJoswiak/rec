#ifndef INCLUDE_CLIENT_TCP_CLIENT_HPP_
#define INCLUDE_CLIENT_TCP_CLIENT_HPP_

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <chrono>
#include <deque>
#include <mutex>

#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"
#include "process/shared_queue.hpp"

class TcpClient {
 public:
  explicit TcpClient(
      std::unordered_map<std::string, std::deque<Command>>& workload,
      std::size_t workload_size);

  // Attempts to open a connection to the host on the given port. Queues initial
  // client messages and spawn reader and writer threads.
  void Start(std::string&& host, unsigned short port);

 private:
  static const int kHeaderSize = 4;

  // Closes the connection to the remote host.
  void Close(int socket_fd);

  // Converts the given endpoint into a sockaddr_storage struct. `ret_addr` and
  // `ret_addrlen` are return parameters which will be populated on success. 
  // Returns true on success.
  bool LookupName(std::string& host, unsigned short port,
      struct sockaddr_storage* ret_addr, std::size_t* ret_addrlen);

  // Creates a socket to the given address and connects to it. Sets newly
  // created file descriptor on `ret_fd` return parameter. Returns true on
  // success.
  bool Connect(const struct sockaddr_storage& addr, const std::size_t& addrlen,
      int* ret_fd);

  // Continuously reads messages from the given file descriptor. The first
  // kHeaderSize bytes of each message must be the size of the message, followed
  // by a Response message (see protobuf definition). After reading response,
  // queues next message to be sent. This function is blocking and should be run
  // on its own thread.
  void Read(int fd);

  // Reads the given number of bytes into buf from the socket with the given
  // file descriptor. Returns true on success.
  bool Read(int fd, char* buf, std::size_t bytes);

  // Continuously writes messages from `out_queue_` to the socket with the given
  // file descriptor. This function is blocking and should be run on its own
  // thread.
  void Write(int fd);

  // Writes the given message to the socket with the given file descriptor.
  // Returns true on success.
  bool Write(int fd, const std::string& message);

  // Returns the next message for the given client in a serialized format.
  std::optional<std::string> GetNextMessage(const std::string& client);

  // Contains pairs of <serialized message, client id>.
  process::common::SharedQueue<std::pair<std::string, std::string>> out_queue_;

  // Map of client address -> time point when the most recent request was sent.
  std::unordered_map<std::string,
      std::chrono::time_point<std::chrono::steady_clock>> send_time_;
  // Map of client address -> deque of commands the client wants to run.
  std::unordered_map<std::string, std::deque<Command>> workload_;

  // Statistics.
  // Number of requests in the workload.
  std::size_t workload_size_;
  // Number of requests read in.
  int num_requests_;
  // Average latency (microseconds);
  double average_latency_;

  std::shared_ptr<spdlog::logger> logger_;
};

#endif  // INCLUDE_CLIENT_TCP_CLIENT_HPP_
