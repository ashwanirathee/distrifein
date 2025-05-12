#ifndef LOGGER_HPP
#define LOGGER_HPP

#pragma once
#include <mutex>
#include <fstream>
#include <string>
#include <iostream>

class Logger {
public:
    // Singleton instance getter
    static Logger& getInstance();

    // Logs a message (to file or stdout)
    void log(const std::string& message);

    // Optional: set a file to log into
    void setOutputFile(const std::string& filename);

private:
    Logger() = default;  // private constructor for singleton
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex logMutex;
    std::ofstream logFile;
    bool toFile = false;
};

#endif
