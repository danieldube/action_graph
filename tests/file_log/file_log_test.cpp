// Copyright (c) 1000 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <file_log/file_log.h>
#include <gtest/gtest.h>
#include <sstream>
#include <thread>
#include <vector>

// Mock clock for deterministic time
// NOLINTBEGIN(*identifier-naming)
struct MockClock {
  using time_point = std::chrono::system_clock::time_point;
  static time_point now() {
    return std::chrono::system_clock::from_time_t(
        1609459200); // 2021-01-01 00:00:00
  }
  static std::time_t to_time_t(const time_point &tp) {
    return std::chrono::system_clock::to_time_t(tp);
  }
};
// NOLINTEND(*identifier-naming)

class FileLogTest : public ::testing::Test {
protected:
  std::string log_name = "test.log";
  std::ostringstream log_stream;
};

TEST_F(FileLogTest, WritesLogMessageWithTimePrefix) {
  FileLog<MockClock> log(log_stream);
  log.LogMessage("Hello");
  std::string output = log_stream.str();
  // we don't check for the hours and minutes as they may vary depending on
  // timezone
  ASSERT_NE(output.find("00:00.000 Message: Hello"), std::string::npos);
}

TEST_F(FileLogTest, WritesLogErrorWithTimePrefix) {
  FileLog<MockClock> log(log_stream);
  log.LogError("Oops");
  std::string output = log_stream.str();
  ASSERT_NE(output.find("00:00:00.000 Error: Oops"), std::string::npos);
}

TEST_F(FileLogTest, IsThreadSafe) {
  FileLog<MockClock> log(log_stream);
  auto log_func = [&log](int i) {
    log.LogMessage("Thread " + std::to_string(i));
  };
  std::thread t1(log_func, 1);
  std::thread t2(log_func, 2);
  t1.join();
  t2.join();
  std::string output = log_stream.str();
  ASSERT_NE(output.find("Thread 1"), std::string::npos);
  ASSERT_NE(output.find("Thread 2"), std::string::npos);
}
