#include <chrono>
#include <iomanip>
#include <sstream>

#include <distrifein/logger.h>

Logger &Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::log(const std::string &message)
{
    std::lock_guard<std::mutex> lock(logMutex);

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()) %
                        1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "] ";
    ss << message;

    if (toFile && logFile.is_open())
    {
        logFile << ss.str() << std::endl;
    }
    else
    {
        std::cout << ss.str() << std::endl;
    }
}

void Logger::setOutputFile(const std::string &filename)
{
    std::lock_guard<std::mutex> lock(logMutex);
    logFile.open(filename, std::ios::out | std::ios::app);
    if (logFile.is_open())
    {
        toFile = true;
    }
}

Logger::~Logger()
{
    if (logFile.is_open())
    {
        logFile.close();
    }
}
