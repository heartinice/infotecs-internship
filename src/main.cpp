#include <logger/logger.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "monitoring/monitoring.h"
#include "multithreading/multithreading.h"

std::atomic<bool>        running(true);
std::mutex               threadsMutex;
std::vector<std::thread> monitoringThreads;

// Функция для выполнения мониторинга
void monitoringTask(SystemMonitorManager& systemMonitorManager, LogLevel logLevel) {
    systemMonitorManager.startMonitoring(logLevel);
}

// Поток для обработки пользовательского ввода
void inputThread(Logger& logger) {
    while (running) {
        std::string command, level;
        std::cout << "Enter a command: (cpu) for CPU, (memory) for Memory, (disk) for Disk, (all) for everything, "
                     "(exit) to stop, (change) to change default log level: ";

        std::cin >> command;

        if (command == "change") {
            std::string newDefaultLogLevel;
            std::cout << "Write new default log level: ";
            std::cin >> newDefaultLogLevel;
            logger.changeLogLevel(Logger::translateLevel(newDefaultLogLevel));
            std::cout << "Default log level was changed\n";
        } else if (command == "exit") {
            running = false;
            break;
        } else if (command != "exit" && command != "cpu" && command != "memory" && command != "disk" &&
                   command != "all" && command != "change") {
            running = false;
            break;
        }

        std::cout << "Enter the message importance level (info, warning, error). Key 'enter' = skip: ";
        std::cin.ignore();
        std::getline(std::cin, level);

        LogLevel logLevel;
        if (level.empty()) {
            logLevel = LogLevel::info;
            std::cout << "Empty level => setted level: info\n";
        } else {
            logLevel = Logger::translateLevel(level);
        }

        if ((logLevel != LogLevel::info) && (logLevel != LogLevel::error) && (logLevel != LogLevel::warning)) {
            running = false;
            break;
        }

        SystemMonitor        systemMonitor(logger);
        SystemMonitorManager systemMonitorManager(systemMonitor, command, logLevel);

        {
            // Создаем новый поток для нового задания
            std::lock_guard<std::mutex> lock(threadsMutex);
            monitoringThreads.emplace_back(monitoringTask, std::ref(systemMonitorManager), logLevel);
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <filename> <LogLevel>\n";
        return -1;
    } else if (argc > 3) {
        std::cerr << "A lot of arguments!\n";
        return -1;
    }

    const std::string initialLevelStr = argv[2], filename = std::string(argv[1]) + ".txt";

    if ((Logger::translateLevel(initialLevelStr) != LogLevel::info) &&
        (Logger::translateLevel(initialLevelStr) != LogLevel::warning) &&
        (Logger::translateLevel(initialLevelStr) != LogLevel::error)) {
        std::cout << "You send unknown level\n";
        running = false;
    }

    Logger logger(filename, initialLevelStr);

    // Поток для обработки ввода пользователя
    std::thread input(inputThread, std::ref(logger));

    input.join();  // Ждем завершения потока ввода

    // Завершаем все потоки мониторинга
    {
        std::lock_guard<std::mutex> lock(threadsMutex);
        for (auto& thread : monitoringThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    return 0;
}

