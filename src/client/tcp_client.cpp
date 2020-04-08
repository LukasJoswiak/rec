#include "client/tcp_client.hpp"

#include <google/protobuf/arena.h>

#include <iomanip>
#include <sstream>
#include <thread>

TcpClient::TcpClient(
    std::unordered_map<std::string, std::deque<Command>>& workload,
    std::size_t workload_size)
    : workload_(workload),
      workload_size_(workload_size) {
  logger_ = spdlog::get("client");
}

void TcpClient::Start(std::string&& host, unsigned short port) {
  struct sockaddr_storage addr;
  std::size_t addrlen;
  if (!LookupName(host, port, &addr, &addrlen)) {
    exit(1);
  }

  int socket_fd;
  if (!Connect(addr, addrlen, &socket_fd)) {
    exit(1);
  }
  logger_->info("connected to {}", host);

  // Read responses on a separate thread.
  std::thread t1([&, socket_fd] () {
    Read(socket_fd);
  });

  // Queue initial message from each client.
  for (const auto& [client, _] : workload_) {
    if (auto serialized = GetNextMessage(client)) {
      out_queue_.push(std::make_pair(*serialized, client));
    }
  }

  // Write messages on a separate thread.
  std::thread t2([&, socket_fd] () {
    Write(socket_fd);
  });

  t1.join();
  t2.join();
}

void TcpClient::Close(int fd) {
  logger_->debug("closing connection");
  close(fd);
}

bool TcpClient::LookupName(std::string& host, unsigned short port,
    struct sockaddr_storage* ret_addr, std::size_t* ret_addrlen) {
  int status;
  struct addrinfo hints;
  struct addrinfo* results;

  memset(&hints, 0, sizeof(hints));
  // IPv4 only for now.
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(host.c_str(), nullptr, &hints, &results)) != 0) {
    logger_->error("getaddrinfo error: {}", gai_strerror(status));
    return false;
  }
  
  // Use the first result from getaddrinfo.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in* v4_addr =
        reinterpret_cast<struct sockaddr_in*>(results->ai_addr);
    v4_addr->sin_port = htons(port);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6* v6_addr =
        reinterpret_cast<struct sockaddr_in6*>(results->ai_addr);
    v6_addr->sin6_port = htons(port);
  } else {
    logger_->error("getaddrinfo failed to provide address");
    freeaddrinfo(results);
    return false;
  }

  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_addrlen = results->ai_addrlen;

  freeaddrinfo(results);
  return true;
}

bool TcpClient::Connect(const struct sockaddr_storage& addr,
    const std::size_t& addrlen, int* ret_fd) {
  int socket_fd;
  if ((socket_fd = socket(addr.ss_family, SOCK_STREAM, 0)) == -1) {
    logger_->error("socket() error: {}", strerror(errno));
    return false;
  }

  // Disable Nagle's algorithm.
  int flag;
  int result = setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (char*) &flag,
                          sizeof(int));
  if (result == -1) {
    logger_->error("failed to enable TCP_NODELAY");
  }

  int res;
  if ((res = connect(socket_fd,
      reinterpret_cast<const sockaddr*>(&addr), addrlen)) == -1) {
    logger_->error("connect() error: {}", strerror(errno));
    return false;
  }

  *ret_fd = socket_fd;
  return true;
}

void TcpClient::Read(int fd) {
  auto start_time = std::chrono::steady_clock::now();
  uint64_t num_received = 0;
  while (1) {
    char sizebuf[kHeaderSize];
    if (!Read(fd, sizebuf, kHeaderSize)) {
      break;
    }

    std::string header(sizebuf, kHeaderSize);
    std::istringstream is(header);
    std::size_t data_size = 0;
    if (!(is >> std::hex >> data_size)) {
      logger_->error("failed to parse header");
      break;
    }

    char buf[data_size];
    if (!Read(fd, buf, data_size)) {
      break;
    }
    auto now = std::chrono::steady_clock::now();
    const std::string data(buf, data_size);
    ++num_received;

    Message message;
    message.ParseFromString(data);

    if (message.type() == Message_MessageType_UNKNOWN) {
      logger_->error("parsed unknown message of size {} with body: {}",
          data_size, data);
      continue;
    }

    if (message.type() == Message_MessageType_RESPONSE) {
      Response r;
      message.message().UnpackTo(&r);

      auto response_time =
          std::chrono::duration_cast<std::chrono::microseconds>(
              now - send_time_[r.client()]);

      logger_->debug("Value ({}:{}, {} microseconds): {}", r.client(),
          r.sequence_number(), response_time.count(), r.value());

      ++num_requests_;
      // Update average latency.
      average_latency_ -= average_latency_ / num_requests_;
      average_latency_ += (double) response_time.count() / num_requests_;

      if (auto serialized = GetNextMessage(r.client())) {
        out_queue_.push(std::make_pair(*serialized, r.client()));
      } else if (num_received == workload_size_) {
        auto elapsed_time = std::chrono::steady_clock::now() - start_time;
        auto elapsed_time_millis =
            std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
        logger_->info("elapsed time to receive {} responses: {} milliseconds",
            num_received, elapsed_time_millis.count());

        int throughput = static_cast<double>(num_received) /
            elapsed_time_millis.count() * 1000;
        logger_->info("average latency: {} microseconds", average_latency_);
        logger_->info("throughput: {} req/s", throughput);

        std::exit(0);
      }
    }
  }
}

bool TcpClient::Read(int fd, char* buf, std::size_t bytes) {
  ssize_t bytes_read = 0;
  while (bytes_read < bytes) {
    ssize_t res = read(fd, buf + bytes_read, bytes - bytes_read);
    if (res == 0) {
      logger_->debug("server disconnected");
      return false;
    }

    if (res == -1) {
      if (errno == EAGAIN || errno == EINTR) {
        continue;
      }
      logger_->error("socket error: {}", strerror(errno));
      return false;
    }
    bytes_read += res;
  }
  return true;
}

void TcpClient::Write(int fd) {
  uint64_t num_sent = 0;
  auto start = std::chrono::steady_clock::now();
  while (1) {
    std::pair<std::string, std::string> pair;
    if (!out_queue_.try_pop(&pair)) {
      // Queue has been shutdown.
      return;
    }

    std::string& message = std::get<0>(pair);
    std::string& client = std::get<1>(pair);
    send_time_[client] = std::chrono::steady_clock::now();
    Write(fd, message);
    ++num_sent;

    if (num_sent == workload_.size()) {
      auto end = std::chrono::steady_clock::now();
      auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      logger_->debug("time to send initial requests to {} clients: {} microseconds", workload_.size(), delta.count());
    }
  }
}

bool TcpClient::Write(int fd, const std::string& message) {
  while (1) {
    ssize_t res = write(fd, message.c_str(), message.size());
    if (res == 0) {
      logger_->error("socket closed prematurely");
      close(fd);
      return false;
    }

    if (res == -1) {
      if (errno == EINTR) {
        continue;
      }
      logger_->error("socket read error: {}", strerror(errno));
      close(fd);
      return false;
    }
    break;
  }
  return true;
}

std::optional<std::string> TcpClient::GetNextMessage(
    const std::string& client) {
  auto& workload = workload_.at(client);
  if (workload.empty()) {
    return std::nullopt;
  }

  std::string name_ = "localhost";

  Command command = workload.front();
  workload.pop_front();

  google::protobuf::Arena arena;

  Request* r = google::protobuf::Arena::CreateMessage<Request>(&arena);
  // r->set_allocated_command(new Command(command));
  r->mutable_command()->set_client(command.client());
  r->mutable_command()->set_sequence_number(command.sequence_number());
  r->mutable_command()->set_key(command.key());
  r->mutable_command()->set_value(command.value());
  r->mutable_command()->set_operation(command.operation());
  r->set_source(name_);

  Message* m = google::protobuf::Arena::CreateMessage<Message>(&arena);
  m->set_type(Message_MessageType_REQUEST);
  m->mutable_message()->PackFrom(*r);
  m->set_from(name_);

  // Build serialized message composed of four-byte message length follewed by
  // the serialized message contents.
  std::ostringstream message_stream;
  std::size_t size = m->ByteSizeLong();
  std::ostringstream header_stream;
  // Write header in hex.
  header_stream << std::setw(kHeaderSize) << std::hex << size;
  if (!header_stream || header_stream.str().size() != kHeaderSize) {
    logger_->error("failed to serialize message size");
    return std::nullopt;
  }
  message_stream << header_stream.str();

  // Serialize and append message.
  std::string serialized;
  m->SerializeToString(&serialized);
  assert(serialized.size() == size);
  message_stream << serialized;
  return message_stream.str();

}

