#include <iostream>
#include <string>
#include <logger/logger.h>
#include "monitoring/monitoring.h"
#include "multithreading/multithreading.h"

int main() {
    std::string filename, level, command;
    std::cout << "Enter the message importance level\n";
    std::cin >> level;
    std::cout << "Enter the log file name\n";
    std::cin >> filename;
    std::cout << "Enter what you want to analyze: cpu, memory, disk, or all\n";
    std::cin >> command;
    


    Logger               logger(filename, level);
    Logger*              pLogger = &logger;
    SystemMonitor        systemMonitor(*pLogger);
    SystemMonitorManager systemMonitorManager(systemMonitor, command, Logger::translateLevel(level));

    while (true) {
        systemMonitorManager.startMonitoring(Logger::translateLevel(level));

        // std::cout << "Enter what you want to analyze: cpu, memory, disk, or all\n";
        // std::cin >> command;
        // std::cin >> level;
    }
}