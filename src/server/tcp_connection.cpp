#include "server/tcp_connection.hpp"

#include <iomanip>
#include <sstream>

// Include the ConnectionManager class explicitly to allow symbol lookup to
// succeed.
#include "server/connection_manager.hpp"

TcpConnection::TcpConnection(int fd,
    std::shared_ptr<ConnectionManager> manager, std::string& endpoint_name)
    : fd_(fd),
      manager_(manager),
      endpoint_(endpoint_name) {
  logger_ = spdlog::get("connection");
}

TcpConnection::~TcpConnection() {
  logger_->trace("shutting down");
  close(fd_);
}

void TcpConnection::Start() {
  logger_->debug("starting connection to {}", endpoint_);

  std::thread t1([&] () {
    Read();
  });

  std::thread t2([&] () {
    Write();
  });

  t1.join();
  t2.join();
}

void TcpConnection::Stop() {
  logger_->debug("stopping connection with {}", endpoint_);
  close(fd_);
}

void TcpConnection::Read() {
  uint64_t num_received = 0;
  while (1) {
    char sizebuf[kHeaderSize];
    if (!Read(fd_, sizebuf, kHeaderSize)) {
      break;
    }

    std::string header(sizebuf, kHeaderSize);
    std::istringstream is(header);
    std::size_t data_size = 0;
    if (!(is >> std::hex >> data_size)) {
      logger_->error("failed to parse header (from: {})", endpoint_);
      break;
    }

    char buf[data_size];
    if (!Read(fd_, buf, data_size)) {
      break;
    }
    const std::string data(buf, data_size);
    ++num_received;

    Message message;
    message.ParseFromString(data);

    if (message.type() == Message_MessageType_UNKNOWN) {
      logger_->error("parsed unknown message of size {} with body: {}",
          data_size, data);
      continue;
    }

    manager_->Handle(message, shared_from_this());
  }
}

bool TcpConnection::Read(int fd, char* buf, std::size_t bytes) {
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

void TcpConnection::Write(const Message& message) {
  if (message.type() == Message_MessageType_UNKNOWN) {
    logger_->error("unknown message type, not writing...");
    return;
  }

  // Write the message size first followed by the message.
  std::ostringstream message_stream;
  std::size_t message_size = message.ByteSizeLong();
  std::ostringstream header_stream;
  header_stream << std::setw(kHeaderSize) << std::hex << message_size;
  if (!header_stream || header_stream.str().size() != kHeaderSize) {
    logger_->error("failed to serialize message size");
    return;
  }
  message_stream << header_stream.str();

  std::string serialized;
  message.SerializeToString(&serialized);
  message_stream << serialized;

  assert(serialized.size() == message_size);
  assert(message_stream.str().size() ==
      header_stream.str().size() + serialized.size());

  out_queue_.push(message_stream.str());
}

void TcpConnection::Write() {
  int num_written = 0;
  while (1) {
    std::string message;
    if (!out_queue_.try_pop(&message)) {
      // Queue has been shutdown.
      return;
    }

    Write(fd_, message);
    ++num_written;
  }
}

bool TcpConnection::Write(int fd, const std::string& message) {
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
