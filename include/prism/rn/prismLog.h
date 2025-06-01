#ifndef PRISM_RN_LOG_H
#define PRISM_RN_LOG_H

#include "ReactCommon/CallInvoker.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <iostream>
#include <jsi/jsi.h>
#include <jsi/jsi.h>
#include <memory>
#include <sstream>
#include <thread>
#include <fmt/core.h>  // 添加这行





using namespace facebook;
namespace prism
{
namespace rn
{


enum class LogLevel {
  Info,
  Warn,
  Error
};

inline std::string levelToString(LogLevel level) {
  switch (level) {
    case LogLevel::Info: return "INFO";
    case LogLevel::Warn: return "WARN";
    case LogLevel::Error: return "ERROR";
    default: return "LOG";
  }
}

inline std::string levelToColor(LogLevel level) {
  switch (level) {
    case LogLevel::Info: return "\033[32m";  // Green
    case LogLevel::Warn: return "\033[33m";  // Yellow
    case LogLevel::Error: return "\033[31m"; // Red
    default: return "\033[0m";
  }
}
inline std::string getTimestamp() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto time = system_clock::to_time_t(now);
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::tm tm;
#if defined(_WIN32)
  localtime_s(&tm, &time);
#else
  localtime_r(&time, &tm);
#endif
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  oss << "." << std::setfill('0') << std::setw(3) << ms.count();  // 添加毫秒部分
  return oss.str();
}


inline std::string getThreadId() {
  std::ostringstream oss;
  oss << std::this_thread::get_id();
  return oss.str();
}


inline std::string formatLog(const std::string& msg, const char* file, int line, LogLevel level) {
  // 只提取文件名
  std::string filename = file;
  auto pos = filename.find_last_of("/\\");
  if (pos != std::string::npos) {
    filename = filename.substr(pos + 1);
  }

  std::ostringstream oss;
  oss << "[" << getTimestamp() << "]";
  oss << "[" << getThreadId() << "]";
  oss << "[" << levelToString(level) << "]";
  oss << "[" << filename << ":" << line << "] ";
  oss << msg;
  return oss.str();
}



inline void logToJS(
  jsi::Runtime& rt,
  const std::string& msg,
  const char* file,
  int line,
  LogLevel level = LogLevel::Info
) {
  std::string fullMsg = formatLog(msg, file, line, level);
  std::string colorMsg = levelToColor(level) + fullMsg + "\033[0m";

  try {
    jsi::Value console = rt.global().getProperty(rt, "console");
    if (console.isObject()) {
      jsi::Object consoleObj = console.asObject(rt);
      jsi::Function logFn = consoleObj.getPropertyAsFunction(rt, "log");
      logFn.call(rt, jsi::String::createFromUtf8(rt, colorMsg));
    }
  } catch (...) {
    std::cerr << "[C++ LOG FALLBACK] " << fullMsg << std::endl;
  }
}


#include <fmt/core.h>  // 添加这行

template <typename... Args>
inline void logToJS(
  jsi::Runtime& rt,
  const char* file,
  int line,
  LogLevel level,
  fmt::format_string<Args...> fmtStr,
  Args&&... args
) {
  std::string msg = fmt::format(fmtStr, std::forward<Args>(args)...);
  logToJS(rt, msg, file, line, level);
}



inline void logToJSAsync(
  jsi::Runtime& rt,
  std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker,
  const std::string& msg,
  const char* file,
  int line,
  LogLevel level = LogLevel::Info
) {
  if (!jsCallInvoker) return;

  std::string fullMsg = formatLog(msg, file, line, level);
  std::string colorMsg = levelToColor(level) + fullMsg + "\033[0m";

  jsCallInvoker->invokeAsync([&rt, colorMsg]() {
    try {
      jsi::Value console = rt.global().getProperty(rt, "console");
      if (console.isObject()) {
        jsi::Object consoleObj = console.asObject(rt);
        jsi::Function logFn = consoleObj.getPropertyAsFunction(rt, "log");
        logFn.call(rt, jsi::String::createFromUtf8(rt, colorMsg));
      }
    } catch (...) {
      std::cerr << "[C++ LOG FALLBACK - JS INVOKER FAILED]" << std::endl;
    }
  });
}
template <typename... Args>
inline void logToJSAsync(
  jsi::Runtime& rt,
  std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker,
  const char* file,
  int line,
  LogLevel level,
  fmt::format_string<Args...> fmtStr,
  Args&&... args
) {
  std::string msg = fmt::format(fmtStr, std::forward<Args>(args)...);
  logToJSAsync(rt, jsCallInvoker, msg, file, line, level);
}




}// namespace rn
}// namespace prism

#define LOG_INFO_F(rt, invoker, fmtStr, ...)                                   \
  prism::rn::logToJSAsync(rt, invoker, __FILE__, __LINE__,                     \
                          prism::rn::LogLevel::Info, FMT_STRING(fmtStr),       \
                          ##__VA_ARGS__)

#define LOG_WARN_F(rt, invoker, fmtStr, ...)                                   \
  prism::rn::logToJSAsync(rt, invoker, __FILE__, __LINE__,                     \
                          prism::rn::LogLevel::Warn, FMT_STRING(fmtStr),       \
                          ##__VA_ARGS__)

#define LOG_ERROR_F(rt, invoker, fmtStr, ...)                                  \
  prism::rn::logToJSAsync(rt, invoker, __FILE__, __LINE__,                     \
                          prism::rn::LogLevel::Error, FMT_STRING(fmtStr),      \
                          ##__VA_ARGS__)


#ifndef NDEBUG
  #define LOG_DEBUG_F(rt, invoker, fmtStr, ...)                               \
    prism::rn::logToJSAsync(rt, invoker, __FILE__, __LINE__,                  \
                            prism::rn::LogLevel::Info, FMT_STRING(fmtStr),    \
                            ##__VA_ARGS__)
#else
  #define LOG_DEBUG_F(rt, invoker, fmtStr, ...)  do {} while(0)
#endif


#endif //PRISM_RN_LOG_H
