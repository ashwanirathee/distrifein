#include <distrifein/logger.hpp>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);

    if (toFile && logFile.is_open()) {
        logFile << message << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

void Logger::setOutputFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFile.open(filename, std::ios::out | std::ios::app);
    if (logFile.is_open()) {
        toFile = true;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}
