#include "console_log.h"

#include <iostream>

namespace action_graph_examples {

ConsoleLog::ConsoleLog(std::ostream &output) : output_(output) {}

void ConsoleLog::LogMessage(const std::string &message) {
  WriteLine("[info] " + message);
}

void ConsoleLog::LogError(const std::string &message) {
  WriteLine("[error] " + message);
}

void ConsoleLog::WriteLine(const std::string &line) {
  std::lock_guard<std::mutex> guard(mutex_);
  output_ << line << std::endl;
}

} // namespace action_graph_examples
