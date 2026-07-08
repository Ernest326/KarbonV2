#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace Karbon {

class Logger {

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    static Logger& get() {
        static Logger instance;
        return instance;
    }

    // Different log levels
    void info(const std::string& message)  { log("INFO", message); }
    void warn(const std::string& message)  { log("WARN", message); }
    void error(const std::string& message) { log("ERROR", message); }
    void debug(const std::string& message) { log("DEBUG", message); }

private:
    Logger() : m_logFile("log.txt", std::ios::app) {}

    static std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    // [TIME] TYPE::MESSAGE
    void log(const char* type, const std::string& message) {
        std::string formattedMessage = "[" + getCurrentTime() + "] " + type + "::" + message;
        std::cout << formattedMessage << std::endl;
        if (m_logFile.is_open()) {
            m_logFile << formattedMessage << std::endl;
        }
    }

    std::ofstream m_logFile;
};

}
