#ifndef MONITORING_H
#define MONITORING_H

#include <string>

#include "../logger/logger.h"

class SystemMonitor {
   public:
    SystemMonitor(Logger& logger);
    void monitorCPU(LogLevel userLogLevel);
    void monitorMemory(LogLevel userLogLevel);
    void monitorDisk(LogLevel userLogLevel);
    void getLoadBoundary(LogLevel userLogLevel);

   private:
    Logger& logger;
};

#endif  // MONITORING_H
