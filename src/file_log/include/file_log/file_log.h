// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_FILE_LOG_H
#define ACTION_GRAPH_FILE_LOG_H
#include <action_graph/log.h>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <ostream>
#include <sstream>

template <typename Clock> class FileLog final : public action_graph::Log {
public:
  explicit FileLog(std::ostream &stream) : stream_(stream), mutex_() {}

  void LogMessage(const std::string &message) override {
    WriteLog("Message: " + message);
  }

  void LogError(const std::string &message) override {
    WriteLog("Error: " + message);
  }

private:
  std::ostream &stream_;
  std::mutex mutex_;

  std::string CurrentTimeString() {
    auto now = Clock::now();
    auto time_t_now = Clock::to_time_t(now);
    std::tm tm_now{};

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%H:%M:%S") << '.' << std::setfill('0')
        << std::setw(3) << ms.count();
    return oss.str();
  }

  void WriteLog(const std::string &line) {
    std::lock_guard<std::mutex> lock(mutex_);
    stream_ << CurrentTimeString() << " " << line << std::endl;
  }
};
#endif // ACTION_GRAPH_FILE_LOG_H
