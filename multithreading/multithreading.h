#ifndef MULTITHREADING_H
#define MULTITHREADING_H

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "../logger/logger.h"
#include "../monitoring/monitoring.h"

class SystemMonitorManager {
   public:
    // SystemMonitorManager(Logger& logger);
    SystemMonitorManager(SystemMonitor& monitor, const std::string& command);
    ~SystemMonitorManager();

    void startMonitoring(LogLevel userLogLevel);
    void stopMonitoring();

   private:
    void monitorCPU(LogLevel userLogLevel);
    void monitorMemory(LogLevel userLogLevel);
    void monitorDisk(LogLevel userLogLevel);

    SystemMonitor            monitor;
    std::vector<std::thread> threads;
    std::atomic<bool>        running;  // Для управления потоками
    std::string              mode;
};

#endif  // MULTITHREADING_H
