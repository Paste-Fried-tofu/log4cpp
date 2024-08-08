#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <list>
#include <vector>
#include <tuple>
#include <ctime>
#include <unordered_map>
#include <iomanip>
#include <functional>
#include <cstdarg>

namespace log4cpp {

using time_point = std::chrono::system_clock::time_point;

class Logger;

class LogLevel{
public:
    enum class Level{
        unknow = 0,
        debug = 1,
        info = 2,
        warn = 3,
        error = 4,
        fatal = 5
    };
    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string& str);
};


class LogEvent{
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, 
            const char* filename, int32_t line, uint32_t elapse,
            std::thread::id thread_id, uint32_t fiber_id, time_point time, 
            const std::string& thread_name);

    const char* getFile() const { return filename_; }
    int32_t getLine() const { return line_; }
    uint32_t getElapse() const { return elapse_; }
    std::thread::id getThreadId() const { return threadId_; }
    const std::string& getThreadName() const { return threadName_; }
    uint32_t getFiberId() const { return fiberId_; }
    time_point getTime() const { return time_; }
    std::string getContent() const { return ss_content_.str(); }
    std::stringstream& getContentStream() { return ss_content_; }
    LogLevel::Level getLevel() const { return level_; }
    std::shared_ptr<Logger> getLogger() const { return logger_; }

    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

private:
    const char* filename_ = nullptr;                       // 文件名
    int32_t line_ = 0;                                     // 行号
    uint32_t elapse_ = 0;                                  // 程序启动到现在的毫秒数
    std::thread::id threadId_;                             // 线程Id
    std::string threadName_;                               // 线程名称
    uint32_t fiberId_ = 0;                                 // 协程Id
    std::chrono::system_clock::time_point time_;           // 时间戳
    std::stringstream ss_content_;                         // 内容
    LogLevel::Level level_;                                // 日志级别
    std::shared_ptr<Logger> logger_;                       // 日志器
};

class LogEventWrap {
public:
    using ptr = std::shared_ptr<LogEventWrap>;
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();

    LogEvent::ptr getEvent() const { return event_; }

    std::stringstream& getSS() { return event_->getContentStream(); }
private:
    LogEvent::ptr event_;
};


class LogFormatter{
public:
    using ptr = std::shared_ptr<LogFormatter>;
    /**
     * @brief 构造函数
     * @param[in] pattern 格式模板
     * @details 
     *  %m 消息
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
    LogFormatter(const std::string& pattern);

public:
    class FormatItem{
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, LogEvent::ptr event) = 0;
        virtual std::string toString() const = 0;
    
    };

    void init();
    std::string format(LogEvent::ptr event);
    std::ostream& format(std::ostream& os, LogEvent::ptr event);

    bool isError() const {return error_;}
    const std::string& getPattern() const { return pattern_; }
private:
    template <typename T>
    std::function<LogFormatter::FormatItem::ptr(const std::string& str)> create_format_item();

private:
    std::string pattern_;                   // 格式模板
    std::vector<FormatItem::ptr> items_;    // 格式项
    bool error_ = false;                    // 是否出错
};


class LogAppender{
friend class Logger;
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() {}
    virtual void log(LogEvent::ptr event) = 0;
    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter() const ;
    bool hasFormatter() const;

protected:
    std::mutex mutex_;
    bool hasFormatter_ = false;
    LogFormatter::ptr formatter_ = nullptr;
};


class StdoutLogAppender : public LogAppender{
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    void log(LogEvent::ptr event) override;
};


class FileLogAppender : public LogAppender{
public:
    using ptr = std::shared_ptr<FileLogAppender>;
    FileLogAppender(const std::string &filename);
    void log(LogEvent::ptr event) override;
    bool reopen();

private:
    std::string filename_;
    std::ofstream ofs_;
    time_point last_time_;
};


class Logger{
friend class LoggerManager;
public:
    using ptr = std::shared_ptr<Logger>;

    Logger(LogLevel::Level level = LogLevel::Level::debug, const std::string& name = "root");

    void log(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppender();

    void addStdoutAppender();
    bool hasStdoutAppender() const;
    void addFileAppender(const std::string& filename);

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(std::string& val);

    const std::string& getName() const { return name_; }
    LogFormatter::ptr getFormatter() const { return formatter_; }
    LogLevel::Level getLevel() const { 
        return level_; 
    }

private:
    static StdoutLogAppender::ptr stdout_appender_;

private:
    std::string name_;                          // 日志名称
    std::list<LogAppender::ptr> appenders_;     // 日志输出器
    LogFormatter::ptr formatter_;               // 日志格式化器
    Logger::ptr root_;                          // 根日志器
    LogLevel::Level level_;                     // 日志级别
    std::mutex mutex_;                           // 线程锁
};

// 单例模式
class LoggerManager{
public:
    static LoggerManager& getInstance(){
        static LoggerManager instance;
        return instance;
    }
    void addLogger(const std::string& name, LogLevel::Level level = LogLevel::Level::debug);
    void addLogger(const std::string& name, const std::string& level="debug");

    Logger::ptr getLogger(const std::string& name);
    Logger::ptr getRoot() const ;

private:
    LoggerManager();
    LoggerManager(const LoggerManager&) = delete;
    LoggerManager& operator=(const LoggerManager&) = delete;

private:
    std::unordered_map<std::string, Logger::ptr> loggers_;
    Logger::ptr root_;

};

#define LOG(logger, level) \
    if(logger->getLevel() <= log4cpp::LogLevel::FromString(level)) \
        log4cpp::LogEventWrap(log4cpp::LogEvent::ptr(new log4cpp::LogEvent(logger, log4cpp::LogLevel::FromString(level), \
                        __FILE__, __LINE__, 0, \
                std::this_thread::get_id(), 0, \
                std::chrono::system_clock::now(), \
                "main"))).getSS()

#define LOG_DEBUG(logger) LOG(logger, "debug")
#define LOG_INFO(logger) LOG(logger, "info")
#define LOG_WARN(logger) LOG(logger, "warn")
#define LOG_ERROR(logger) LOG(logger, "error")
#define LOG_FATAL(logger) LOG(logger, "fatal")

#define debug() LOG_DEBUG(log4cpp::LoggerManager::getInstance().getRoot())
#define info() LOG_INFO(log4cpp::LoggerManager::getInstance().getRoot())
#define warn() LOG_WARN(log4cpp::LoggerManager::getInstance().getRoot())
#define error() LOG_ERROR(log4cpp::LoggerManager::getInstance().getRoot())
#define fatal() LOG_FATAL(log4cpp::LoggerManager::getInstance().getRoot())


#define LOG_FMT(logger, level, fmt, ...) \
    if(logger->getLevel() <= log4cpp::LogLevel::FromString(level))\
        log4cpp::LogEventWrap(log4cpp::LogEvent::ptr(new log4cpp::LogEvent(logger, log4cpp::LogLevel::FromString(level), \
                        __FILE__, __LINE__, 0, \
                std::this_thread::get_id(), 0, \
                std::chrono::system_clock::now(), \
                "main"))).getEvent()->format(fmt, __VA_ARGS__)


inline Logger::ptr simple_init(std::string logger_name = "root", std::string type = "stdout"){
    auto &lm = LoggerManager::getInstance();
    if(logger_name == "root"){
        try
        {
            if(type == "stdout" && !lm.getRoot()->hasStdoutAppender()){
                lm.getRoot()->addStdoutAppender();
                return lm.getRoot();
            }
            else if(type == "file"){
                lm.getRoot()->addFileAppender("log.txt");
                return lm.getRoot();
            }
            else{
                throw std::runtime_error("unknown log type");
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    } else {
        auto logger = lm.getLogger(logger_name);
        try{
            if(type == "stdout" && !logger->hasStdoutAppender()){
                logger->addStdoutAppender();
            }
            else if(type == "file"){
                logger->addFileAppender("log.txt");
            }
            else{
                throw std::runtime_error("unknown log type");
            }
        }
        catch(const std::exception& e){
            std::cerr << e.what() << '\n';
        }
        return logger;
    }
    return nullptr;
}


}// namespace log4cpp

#endif // __LOG_HPP__