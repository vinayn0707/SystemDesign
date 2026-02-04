#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace MovieBooking {
namespace Utils {

// Log levels
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

// Log entry structure
struct LogEntry {
    LogLevel level;
    std::string message;
    std::string category;
    std::chrono::system_clock::time_point timestamp;
    std::thread::id threadId;
    std::string file;
    int line;
    std::string function;
    
    LogEntry(LogLevel lvl, const std::string& msg, const std::string& cat = "",
             const std::string& file = "", int line = 0, const std::string& function = "")
        : level(lvl), message(msg), category(cat), timestamp(std::chrono::system_clock::now()),
          threadId(std::this_thread::get_id()), file(file), line(line), function(function) {}
};

// Log formatter interface
class ILogFormatter {
public:
    virtual ~ILogFormatter() = default;
    virtual std::string format(const LogEntry& entry) = 0;
};

// Default log formatter
class DefaultLogFormatter : public ILogFormatter {
private:
    bool includeThreadId_;
    bool includeFileInfo_;
    std::string dateFormat_;
    
public:
    explicit DefaultLogFormatter(bool includeThreadId = true, bool includeFileInfo = false,
                                const std::string& dateFormat = "%Y-%m-%d %H:%M:%S");
    
    std::string format(const LogEntry& entry) override;
    
    void setIncludeThreadId(bool include) { includeThreadId_ = include; }
    void setIncludeFileInfo(bool include) { includeFileInfo_ = include; }
    void setDateFormat(const std::string& format) { dateFormat_ = format; }

private:
    std::string levelToString(LogLevel level) const;
    std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) const;
    std::string extractFileName(const std::string& path) const;
};

// JSON log formatter
class JsonLogFormatter : public ILogFormatter {
public:
    std::string format(const LogEntry& entry) override;

private:
    std::string escapeJsonString(const std::string& str) const;
    std::string levelToString(LogLevel level) const;
};

// Log appender interface
class ILogAppender {
public:
    virtual ~ILogAppender() = default;
    virtual void append(const LogEntry& entry) = 0;
    virtual void flush() = 0;
    virtual bool isReady() const = 0;
};

// Console appender
class ConsoleAppender : public ILogAppender {
private:
    std::unique_ptr<ILogFormatter> formatter_;
    mutable std::mutex mutex_;
    
public:
    explicit ConsoleAppender(std::unique_ptr<ILogFormatter> formatter = nullptr);
    
    void append(const LogEntry& entry) override;
    void flush() override;
    bool isReady() const override { return true; }
};

// File appender
class FileAppender : public ILogAppender {
private:
    std::string filename_;
    std::unique_ptr<ILogFormatter> formatter_;
    std::ofstream fileStream_;
    size_t maxFileSize_;
    int maxBackupFiles_;
    mutable std::mutex mutex_;
    
public:
    explicit FileAppender(const std::string& filename,
                         std::unique_ptr<ILogFormatter> formatter = nullptr,
                         size_t maxFileSize = 10 * 1024 * 1024, // 10MB
                         int maxBackupFiles = 5);
    
    ~FileAppender();
    
    void append(const LogEntry& entry) override;
    void flush() override;
    bool isReady() const override;
    
    // Configuration
    void setMaxFileSize(size_t size) { maxFileSize_ = size; }
    void setMaxBackupFiles(int count) { maxBackupFiles_ = count; }

private:
    void openFile();
    void rotateFile();
    std::string generateBackupFilename(int backupNumber) const;
};

// Async appender for non-blocking logging
class AsyncAppender : public ILogAppender {
private:
    std::unique_ptr<ILogAppender> underlyingAppender_;
    std::queue<LogEntry> logQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    std::atomic<bool> running_;
    std::thread workerThread_;
    size_t maxQueueSize_;
    
public:
    explicit AsyncAppender(std::unique_ptr<ILogAppender> appender, size_t maxQueueSize = 10000);
    ~AsyncAppender();
    
    void append(const LogEntry& entry) override;
    void flush() override;
    bool isReady() const override;
    
    void start();
    void stop();

private:
    void workerFunction();
};

// Logger class
class Logger {
private:
    std::string name_;
    LogLevel minLevel_;
    std::vector<std::unique_ptr<ILogAppender>> appenders_;
    mutable std::shared_mutex appendersMutex_;
    bool asyncMode_;
    
public:
    explicit Logger(const std::string& name, LogLevel minLevel = LogLevel::INFO,
                    bool asyncMode = false);
    
    ~Logger();
    
    // Configuration
    void setMinLevel(LogLevel level) { minLevel_ = level; }
    LogLevel getMinLevel() const { return minLevel_; }
    void setAsyncMode(bool async) { asyncMode_ = async; }
    
    // Appender management
    void addAppender(std::unique_ptr<ILogAppender> appender);
    void removeAppender(size_t index);
    void clearAppenders();
    size_t getAppenderCount() const;
    
    // Logging methods
    void log(LogLevel level, const std::string& message, const std::string& category = "",
             const std::string& file = "", int line = 0, const std::string& function = "");
    
    void trace(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "");
    
    void debug(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "");
    
    void info(const std::string& message, const std::string& category = "",
              const std::string& file = "", int line = 0, const std::string& function = "");
    
    void warn(const std::string& message, const std::string& category = "",
              const std::string& file = "", int line = 0, const std::string& function = "");
    
    void error(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "");
    
    void fatal(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "");
    
    // Utility methods
    void flush();
    bool isLevelEnabled(LogLevel level) const { return level >= minLevel_; }

private:
    void writeToAppenders(const LogEntry& entry);
};

// Logger manager (singleton)
class LoggerManager {
private:
    static std::unique_ptr<LoggerManager> instance_;
    static std::mutex instanceMutex_;
    
    std::unordered_map<std::string, std::unique_ptr<Logger>> loggers_;
    mutable std::shared_mutex loggersMutex_;
    LogLevel globalMinLevel_;
    bool asyncMode_;
    
    LoggerManager();
    
public:
    ~LoggerManager();
    
    static LoggerManager& getInstance();
    static void shutdown();
    
    // Logger management
    Logger& getLogger(const std::string& name);
    void removeLogger(const std::string& name);
    void removeAllLoggers();
    
    // Global configuration
    void setGlobalMinLevel(LogLevel level);
    void setAsyncMode(bool async);
    
    // Convenience methods for root logger
    void log(LogLevel level, const std::string& message, const std::string& category = "",
             const std::string& file = "", int line = 0, const std::string& function = "");
    
    void trace(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "");
    
    void debug(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "");
    
    void info(const std::string& message, const std::string& category = "",
              const std::string& file = "", int line = 0, const std::string& function = "");
    
    void warn(const std::string& message, const std::string& category = "",
              const std::string& file = "", int line = 0, const std::string& function = "");
    
    void error(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "");
    
    void fatal(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "");
    
    void flushAll();

private:
    Logger& getOrCreateLogger(const std::string& name);
    void configureDefaultLogger();
};

// Convenience macros for logging
#define LOG_TRACE(msg) MovieBooking::Utils::LoggerManager::getInstance().trace(msg, "", __FILE__, __LINE__, __FUNCTION__)
#define LOG_DEBUG(msg) MovieBooking::Utils::LoggerManager::getInstance().debug(msg, "", __FILE__, __LINE__, __FUNCTION__)
#define LOG_INFO(msg) MovieBooking::Utils::LoggerManager::getInstance().info(msg, "", __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARN(msg) MovieBooking::Utils::LoggerManager::getInstance().warn(msg, "", __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR(msg) MovieBooking::Utils::LoggerManager::getInstance().error(msg, "", __FILE__, __LINE__, __FUNCTION__)
#define LOG_FATAL(msg) MovieBooking::Utils::LoggerManager::getInstance().fatal(msg, "", __FILE__, __LINE__, __FUNCTION__)

#define LOG_TRACE_CAT(cat, msg) MovieBooking::Utils::LoggerManager::getInstance().trace(msg, cat, __FILE__, __LINE__, __FUNCTION__)
#define LOG_DEBUG_CAT(cat, msg) MovieBooking::Utils::LoggerManager::getInstance().debug(msg, cat, __FILE__, __LINE__, __FUNCTION__)
#define LOG_INFO_CAT(cat, msg) MovieBooking::Utils::LoggerManager::getInstance().info(msg, cat, __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARN_CAT(cat, msg) MovieBooking::Utils::LoggerManager::getInstance().warn(msg, cat, __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR_CAT(cat, msg) MovieBooking::Utils::LoggerManager::getInstance().error(msg, cat, __FILE__, __LINE__, __FUNCTION__)
#define LOG_FATAL_CAT(cat, msg) MovieBooking::Utils::LoggerManager::getInstance().fatal(msg, cat, __FILE__, __LINE__, __FUNCTION__)

// Scoped logger for specific categories
class ScopedLogger {
private:
    Logger& logger_;
    std::string category_;
    
public:
    ScopedLogger(const std::string& loggerName, const std::string& category)
        : logger_(LoggerManager::getInstance().getLogger(loggerName)), category_(category) {}
    
    void trace(const std::string& message) { logger_.trace(message, category_); }
    void debug(const std::string& message) { logger_.debug(message, category_); }
    void info(const std::string& message) { logger_.info(message, category_); }
    void warn(const std::string& message) { logger_.warn(message, category_); }
    void error(const std::string& message) { logger_.error(message, category_); }
    void fatal(const std::string& message) { logger_.fatal(message, category_); }
};

} // namespace Utils
} // namespace MovieBooking
