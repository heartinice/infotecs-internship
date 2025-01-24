#include "multithreading.h"

#include <chrono>
#include <iostream>
#include <string>

#include "../monitoring/monitoring.h"

SystemMonitorManager::SystemMonitorManager(SystemMonitor& monitor, const std::string& command, LogLevel userLogLevel)
    : monitor(monitor), running(false), mode(command), userLogLevel(userLogLevel) {}

SystemMonitorManager::~SystemMonitorManager() {
    stopMonitoring();  // Остановка потоков при уничтожении объекта
}

// Запуск мониторинга
void SystemMonitorManager::startMonitoring(LogLevel userLogLevel) {
    if (running) {  // Если мониторинг уже запущен, ничего не делаем
        return;
    }

    running = true;  // Устанавливаем флаг
    if (mode == "all") {
        threads.emplace_back([&]() { monitorCPU(userLogLevel); });
        threads.emplace_back([&]() { monitorMemory(userLogLevel); });
        threads.emplace_back([&]() { monitorDisk(userLogLevel); });
    } else if (mode == "cpu") {
        threads.emplace_back([&]() { monitorCPU(userLogLevel); });
    } else if (mode == "memory") {
        threads.emplace_back([&]() { monitorMemory(userLogLevel); });
    } else if (mode == "disk") {
        threads.emplace_back([&]() { monitorDisk(userLogLevel); });
    } else {
        std::cerr << "Invalid mode. Use 'all', 'cpu', 'memory', or 'disk'." << std::endl;
        running = false;
        return;
    }
}

// Остановка мониторинга
void SystemMonitorManager::stopMonitoring() {
    if (!running) {
        return;
    }

    running = false;  // Устанавливаем флаг остановки
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();  // Дожидаемся завершения потока
        }
    }

    threads.clear();  // Очищаем список потоков
}

void SystemMonitorManager::monitorCPU(LogLevel userLogLevel) {
    while (running) {
        monitor.monitorCPU(userLogLevel);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void SystemMonitorManager::monitorMemory(LogLevel userLogLevel) {
    while (running) {
        monitor.monitorMemory(userLogLevel);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void SystemMonitorManager::monitorDisk(LogLevel userLogLevel) {
    while (running) {
        monitor.monitorDisk(userLogLevel);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

// int main() {
//     Logger               logger("log.txt", "info");
//     SystemMonitor        n1(logger);
//     SystemMonitorManager t1(n1, "all");
//     t1.startMonitoring();

//     return 0;
// }