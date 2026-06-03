#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include <time>

namespace Karbon {

class Logger {

public:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    static Logger& Get() {
        static Logger instance;
        return instance;
    }

    const char* messageTemplate = "[%s] %s::%s"; // [TIME] TYPE::MESSAGE

    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    struct LogTemplate {
        std::string time;
        std::string type;
        std::string message;
    }

    // Log the message using the provided template
    void log(LogTemplate logTemplate) {
        std::string formattedMessage = std::string(messageTemplate)
            .replace(logTemplate.time, "%s")
            .replace(logTemplate.type, "%s")
            .replace(logTemplate.message, "%s");
        std::cout << formattedMessage << std::endl;
        writeToFile(formattedMessage);
    }

    // Different log levels
    void info(const std::string& message) {
        LogTemplate template;
        template.time = getCurrentTime();
        template.type = "INFO";
        template.message = message;
        log(template);
    }

    void warn(const std::string& message) {
        LogTemplate template;
        template.time = getCurrentTime();
        template.type = "WARN";
        template.message = message;
        log(template);
    }

    void error(const std::string& message) {
        LogTemplate template;
        template.time = getCurrentTime();
        template.type = "ERROR";
        template.message = message;
        log(template);
    }

    void debug(const std::string& message) {
        LogTemplate template;
        template.time = getCurrentTime();
        template.type = "DEBUG";
        template.message = message;
        log(template);
    }

    // Write log to file
    void writeToFile(const std::string& message) {
        std::fstream logFile("log.txt", std::ios::app);
        if (logFile.is_open()) {
            logFile << message << std::endl;
            logFile.close();
        }
    }
};

}