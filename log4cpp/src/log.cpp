#include "log.hpp"

namespace log4cpp{

const char * LogLevel::ToString(LogLevel::Level level)
{
    switch (level){
#define XX(name) \
    case LogLevel::Level::name:\
        return #name;
    XX(debug)
    XX(info)
    XX(warn)
    XX(error)
    XX(fatal)
#undef XX
    default: return "unknow";
    }
}

LogLevel::Level LogLevel::FromString(const std::string &str)
{
#define XX(level, v) \
    if(str == #v)\
        return LogLevel::Level::level;
    XX(debug, debug)
    XX(info, info)
    XX(warn, warn)
    XX(error, error)
    XX(fatal, fatal)
    XX(debug, DEBUG)
    XX(info, INFO)
    XX(warn, WARN)
    XX(error, ERROR)
    XX(fatal, FATAL)
    return LogLevel::Level::unknow;
#undef XX
}

class MessageFormatItem : public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getContent();
    }
    std::string toString() const override {
        return "message";
    }
};

class LevelFormatItem : public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << LogLevel::ToString(event->getLevel());
    }

    std::string toString() const override {
        return "level";
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getElapse();
    }

    std::string toString() const override {
        return "elapse";
    }
};

class NameFormatItem : public LogFormatter::FormatItem{
public:
    NameFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
    std::string toString() const override {
        return "name";
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
    std::string toString() const override {
        return "threadId";
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
    std::string toString() const override {
        return "FiberId";
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
    std::string toString() const override {
        return "threadName";
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") : format_(format){
        if(format_.empty()){
            format_ = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        using namespace std::chrono;
        auto tp = event->getTime();
        std::time_t t = system_clock::to_time_t(tp);
        std::tm *tm = std::localtime(&t);
        os << std::put_time(tm, format_.c_str());

    }
    std::string toString() const override {
        return "time";
    }

private:
    std::string format_;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFile();
    }
    std::string toString() const override {
        return "filename";
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLine();
    }
    std::string toString() const override {
        return "line";
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << std::endl;
    }
    std::string toString() const override {
        return "newline";
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :string_(str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << string_;
    }
    std::string toString() const override {
        return "string";
    }
private:
    std::string string_;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << "\t";
    }
    std::string toString() const override {
        return "tab";
    }
};

LogFormatter::LogFormatter(const std::string &pattern) : pattern_(pattern)
{
    init();
}

template <typename T>
std::function<LogFormatter::FormatItem::ptr(const std::string& str)> LogFormatter::create_format_item() {
    return [](const std::string& fmt) { return LogFormatter::FormatItem::ptr(new T(fmt)); };
}

// %x  %x{xxx}  %%
void LogFormatter::init()
{
    // string, format, type
    std::vector<std::tuple<std::string, std::string, int>> items;
    std::string normal_str;
    for(std::size_t i=0; i<pattern_.size(); ++i){
        if(pattern_[i] != '%'){
            normal_str += pattern_[i];
            continue;
        }

        if(i+1 < pattern_.size() && pattern_[i+1] == '%'){
            normal_str += pattern_[i+1];
            continue;
        }

        int flag = 0;
        size_t str_begin = ++i;
        size_t fmt_begin = i;
        std::string str;
        std::string fmt_str;
        while(i < pattern_.size()){
            if(!flag && !std::isalpha(pattern_[i]) && pattern_[i] != '{' && pattern_[i] != '}') {
                if(str.empty()){
                    str = pattern_.substr(str_begin, i - str_begin);
                }
                --i;
                break;
            }
            if(flag == 0 && pattern_[i] == '{'){
                str = pattern_.substr(str_begin, i - str_begin);
                flag = 1;
                fmt_begin = ++i;
                continue;
            }
            else if(flag == 1 && pattern_[i] == '}'){
                fmt_str = pattern_.substr(fmt_begin, i - fmt_begin);
                flag = 0;
                ++i;
                continue;
            }
            ++i;
            if(i == pattern_.size() && str.empty()){
                str = pattern_.substr(str_begin);
            }
        }
        if(flag == 0){
            if(!normal_str.empty()){
                items.emplace_back(normal_str, std::string(), 0);
                normal_str.clear();
            }
            items.emplace_back(str, fmt_str, 1);
        }
        else if(flag == 1){
            std::cout << "pattern pharse error: " << pattern_ << "-" << pattern_.substr(i) << std::endl;
            error_ = true;
            items.emplace_back("<<pattern error>>", fmt_str, 0);
        }
    }

    if(!normal_str.empty()){
        items.emplace_back(normal_str, std::string(), 0);
    }
    // 使用函数模板填充map
    static std::unordered_map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
        {"m", create_format_item<MessageFormatItem>()},           //m:消息
        {"p", create_format_item<LevelFormatItem>()},             //p:日志级别
        {"r", create_format_item<ElapseFormatItem>()},            //r:累计毫秒数
        {"c", create_format_item<NameFormatItem>()},              //c:日志名称
        {"t", create_format_item<ThreadIdFormatItem>()},          //t:线程id
        {"n", create_format_item<NewLineFormatItem>()},           //n:换行
        {"d", create_format_item<DateTimeFormatItem>()},          //d:时间
        {"f", create_format_item<FilenameFormatItem>()},          //f:文件名
        {"l", create_format_item<LineFormatItem>()},              //l:行号
        {"T", create_format_item<TabFormatItem>()},               //T:Tab
        {"F", create_format_item<FiberIdFormatItem>()},           //F:协程id
        {"N", create_format_item<ThreadNameFormatItem>()},        //N:线程名称
    };


    for(auto& i : items) {
        if(std::get<2>(i) == 0) {
            items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end()) {
                items_.push_back(FormatItem::ptr{new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")});
                error_ = true;
            } else {
                items_.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

std::string LogFormatter::format(LogEvent::ptr event)
{
    std::stringstream ss;
    for(auto &i : items_){
        i->format(ss, event);
    }
    return ss.str();
}

std::ostream &LogFormatter::format(std::ostream &os, LogEvent::ptr event)
{
    for(auto &i : items_){
        i->format(os, event);
    }
    return os;
}

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, 
    const char* filename, int32_t line, uint32_t elapse,
    std::thread::id thread_id, uint32_t fiber_id, std::chrono::system_clock::time_point time, 
    const std::string& thread_name):
    logger_(logger),
    level_(level),
    filename_(filename),
    line_(line),
    elapse_(elapse),
    threadId_(thread_id),
    fiberId_(fiber_id),
    time_(time),
    threadName_(thread_name){

}

void LogEvent::format(const char *fmt, ...)
{
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
    // Compute the size of the buffer we need
    va_list al_copy;
    va_copy(al_copy, al);
    int len = vsnprintf(nullptr, 0, fmt, al_copy);
    va_end(al_copy);

    if (len < 0) {
        return;
    }

    size_t size = static_cast<size_t>(len) + 1;
    char* buf = new char[size];

    vsnprintf(buf, size, fmt, al);

    ss_content_ << std::string(buf, len);
    delete[] buf;
}

Logger::Logger(LogLevel::Level level, const std::string & name) : name_(name), level_(level)
{
    formatter_.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::log(LogEvent::ptr event)
{
    mutex_.lock();
    if(level_ <= event->getLevel()){
        if(!appenders_.empty())
            for(auto& i : appenders_){
                i->log(event);
            }
        else if(root_){
            root_->log(event);
        }
    }
    mutex_.unlock();
}

void Logger::addAppender(LogAppender::ptr appender)
{
    mutex_.lock();
    if(!appender->getFormatter()){
        appender->setFormatter(formatter_);
    }
    appenders_.emplace_back(appender);
    mutex_.unlock();
}

void Logger::delAppender(LogAppender::ptr appender)
{
    mutex_.lock();
    for(auto it = appenders_.begin(); it != appenders_.end(); ++it){
        if(*it == appender){
            appenders_.erase(it);
            break;
        }
    }
    mutex_.unlock();
}

void Logger::clearAppender()
{
    mutex_.lock();
    appenders_.clear();
    mutex_.unlock();
}

void Logger::addStdoutAppender()
{
    addAppender(stdout_appender_);
}

bool Logger::hasStdoutAppender() const
{
    for(auto& i : appenders_){
        if(dynamic_cast<StdoutLogAppender*>(i.get())){
            return true;
        }
    }
    return false;
}

void Logger::addFileAppender(const std::string &filename)
{
    addAppender(std::make_shared<FileLogAppender>(filename));
}

void Logger::setFormatter(LogFormatter::ptr val)
{
    formatter_ = val;
    for(auto& i : appenders_){
        if(!i->hasFormatter()){
            i->setFormatter(formatter_);
        }
    }
}

void Logger::setFormatter(std::string &val)
{
    LogFormatter::ptr new_val(new LogFormatter(val));
    if(new_val->isError()){
        std::cout << "Logger setFormatter name=" << name_ << " value=" << val << " invalid formatter" << std::endl;
        return;
    }
    formatter_ = new_val;
    for(auto& i : appenders_){
        if(!i->hasFormatter()){
            i->setFormatter(formatter_);
        }
    }
}

void LogAppender::setFormatter(LogFormatter::ptr val)
{
    if(!formatter_) hasFormatter_ = true;
    formatter_ = val;
}

LogFormatter::ptr LogAppender::getFormatter() const
{
    return formatter_;
}

bool LogAppender::hasFormatter() const
{
    return hasFormatter_;
}

LogEventWrap::LogEventWrap(LogEvent::ptr e) : event_(e)
{
}

LogEventWrap::~LogEventWrap()
{
    event_->getLogger()->log(event_);
}

void StdoutLogAppender::log(LogEvent::ptr event)
{
    mutex_.lock();
    formatter_->format(std::cout, event);
    mutex_.unlock();
}

FileLogAppender::FileLogAppender(const std::string &filename) : filename_(filename)
{
    reopen();
}

void FileLogAppender::log(LogEvent::ptr event)
{
    auto now = event->getTime();
    if(now != last_time_){
        reopen();
        last_time_ = now;
    }
    mutex_.lock();
    if(!formatter_->format(ofs_, event)){
        std::cout << "LogAppender[" << filename_ << "] format error" << std::endl;
    }
    mutex_.unlock();
}

bool FileLogAppender::reopen()
{
    if(ofs_.is_open()){
        ofs_.close();
    }
    ofs_.open(filename_, std::ios::app);
    return ofs_.is_open();
}

void LoggerManager::addLogger(const std::string &name, LogLevel::Level level)
{
    if(loggers_.find(name) != loggers_.end()){
        return;
    }
    else {
        loggers_[name] = std::make_shared<Logger>(level, name);
    }
}

void LoggerManager::addLogger(const std::string &name, const std::string &level)
{
    if(loggers_.find(name) != loggers_.end()){
        return;
    }
    else {
        loggers_[name] = std::make_shared<Logger>(LogLevel::FromString(level), name);
    }
}

Logger::ptr LoggerManager::getLogger(const std::string &name)
{
    auto it = loggers_.find(name);
    if(it == loggers_.end()){
        this->addLogger(name, LogLevel::Level::debug);
        it = loggers_.find(name);
        return it->second;
    }
    return it->second;
}

Logger::ptr LoggerManager::getRoot() const
{
    return root_;
}

LoggerManager::LoggerManager() : root_(std::make_shared<Logger>())
{
    loggers_[root_->getName()] = root_;
}

StdoutLogAppender::ptr Logger::stdout_appender_ = std::make_shared<StdoutLogAppender>();





} // namespace YcServer
