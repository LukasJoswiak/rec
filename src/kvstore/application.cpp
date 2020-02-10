#include "kvstore/application.hpp"

namespace kvstore {

Application::Application() {
  logger_ = spdlog::stdout_color_mt("application");
}

std::optional<Response> Application::Execute(const Command& command) {
  if (!Executed(command)) {
    // Find the highest sequence number for the client the command is from.
    int sequence_number = 1;
    if (sequence_numbers_.find(command.client()) != sequence_numbers_.end()) {
      sequence_number = sequence_numbers_.at(command.client());
    }

    if (command.sequence_number() != sequence_number) {
      logger_->warn(
          "command has sequence number {}, expected sequence number {}",
          command.sequence_number(), sequence_number);
      return std::nullopt;
    }

    // Execute the command and get the result.
    std::string result;
    switch (command.operation()) {
      case Command_Operation_GET:
        result = store_.Get(command.key());
        break;
      case Command_Operation_PUT:
        result = store_.Put(command.key(), command.value());
        break;
      // APPEND not yet supported.
      default:
        logger_->error("unrecognized command");
        result = "";
    }

    Response r;
    r.set_sequence_number(sequence_number);
    r.set_value(result);
    // TODO: Remove at some point... for closed loop client testing only.
    r.set_client(command.client());

    sequence_numbers_[command.client()] = sequence_number + 1;
    results_[command.client()] = r;
  } else {
    logger_->trace("command has already been executed");
  }

  return results_[command.client()];
}

bool Application::Executed(const Command& command) {
  if (sequence_numbers_.find(command.client()) == sequence_numbers_.end()) {
    return false;
  }
  return command.sequence_number() < sequence_numbers_[command.client()];
}

}  // namespace kvstore
