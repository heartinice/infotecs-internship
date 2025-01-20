#include "logger.h"

Logger::Logger(const string &filename, const string &level)
    : defaultLevel(translateLevel(level)), logFile(filename, ios::app) {
    // Проверяем доступность файла при инициализации
    if (!logFile) {
        throw std::runtime_error("Unable to open log file: " + filename);
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

LogLevel Logger::translateLevel(const string &level) {
    for (char c : level) {
        c = std::tolower(c);
    }

    if (level == "info") return LogLevel::info;
    if (level == "warning") return LogLevel::warning;
    if (level == "error") return LogLevel::error;

    return LogLevel::unknown;
}

void Logger::saveMessage(const string &message, LogLevel currentLevel) {
    if (defaultLevel > currentLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(logMutex);

    logFile << "[" << getcurrentTime() << "]" << "[" << Leveltostring(currentLevel) << "]" << message << endl;
}
void Logger::changeLogLevel(LogLevel newdefLevel) { defaultLevel = newdefLevel; }

string Logger::getcurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);  // Конвертируем в локальное время
    std::tm *localTime = std::localtime(&currentTime);
    // Форматируем время в строку
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

string Logger::Leveltostring(LogLevel currentLevel) {
    switch (currentLevel) {
        case LogLevel::info:
            return "INFO";
        case LogLevel::warning:
            return "WARNING";
        case LogLevel::error:
            return "ERROR";
        case LogLevel::unknown:
        default:
            return "UNKNOWN";
    }
}

bool Logger::hasError() const { return errorOccurred; }
