#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

using namespace std;

enum LogLevel {
    unknown = 0,  // неизвестное
    info    = 1,  // информация
    warning = 2,  // предупреждение
    error   = 3   // ошибка
};

class Logger {
   public:
    Logger(const string &filename, const string &defaultLevel);  // конструктор инициализации библиотеки

    ~Logger();  // деструктор

    void            saveMessage(const string &message, LogLevel currentLevel);
    void            changeLogLevel(LogLevel newdefLevel);
    string          Leveltostring(LogLevel currentLevel);
    bool            hasError() const;
    static LogLevel translateLevel(const string &level);
    string          getcurrentTime();

   private:
    string   filename;
    LogLevel defaultLevel;
    // std::ofstream logFile;
    // std::mutex logMutex;

    std::queue<std::string> logQueue;      // Очередь сообщений для записи
    std::mutex              logMutex;      // Мьютекс для защиты очереди
    std::condition_variable logCondition;  // Условная переменная
    std::ofstream           logFile;       // Файловый поток для записи логов
    bool                    isRunning;     // Флаг работы логгера
    std::thread             logThread;     // Фоновый поток для записи логов

    bool errorOccurred = false;
};

#endif  // LOGGER_H