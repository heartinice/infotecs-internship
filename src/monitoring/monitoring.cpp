#include "monitoring.h"

#include <sys/statvfs.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

SystemMonitor::SystemMonitor(Logger& logger) : logger(logger) {}

int loadmin;
int loadmax;

void SystemMonitor::writeToOutputFile(const std::string& message){
    std::ofstream outFile("output_app.txt", std::ios::app); // Открытие файла для дозаписи
    if (outFile) {
        outFile << message << "\n"; // Записываем сообщение в файл
    }
}

void SystemMonitor::monitorCPU(LogLevel userLogLevel) {
    static long prevIdleTime = 0, prevTotalTime = 0;  // Храним предыдущие значения для расчета

    // Чтение файла /proc/stat
    std::ifstream statFile("/proc/stat");
    if (!statFile) {
        logger.saveMessage("Failed to open /proc/stat", LogLevel::error);
        return;
    }

    std::string line;
    if (std::getline(statFile, line) && line.find("cpu") == 0) {  // Первая строка с "cpu"
        std::istringstream iss(line);
        std::string        cpuLabel;
        long               user, nice, system, idle, iowait, irq, softirq, steal;

        // Считываем значения
        iss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        // Вычисляем суммарное время простоя и общее время
        long idleTime  = idle + iowait;  // Idle = idle + iowait
        long totalTime = user + nice + system + idleTime + irq + softirq + steal;

        // Рассчитываем разницы
        long deltaIdle  = idleTime - prevIdleTime;
        long deltaTotal = totalTime - prevTotalTime;

        // Обновляем предыдущие значения для следующего цикла
        prevIdleTime  = idleTime;
        prevTotalTime = totalTime;

        // Проверяем, чтобы избежать деления на 0
        if (deltaTotal > 0) {
            double cpuLoad = 100.0 * (deltaTotal - deltaIdle) / deltaTotal;
            cpuLoad        = std::round(cpuLoad * 100) / 100.0;

            getLoadBoundary(userLogLevel);

            if (loadmin <= cpuLoad && cpuLoad <= loadmax) {
                // std::cout << logger.getcurrentTime() << logger.Leveltostring(userLogLevel)
                //           << " Average CPU Load: " << cpuLoad;
                std::ostringstream oss;
                oss << "Average CPU Load: " << cpuLoad << "%";
                writeToOutputFile(oss.str()); // Сохраняем в файл
            }

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << cpuLoad;

            logger.saveMessage(" Average CPU Load: " + oss.str() + "%", getLevelfromBound(cpuLoad));

        } else {
            logger.saveMessage(" CPU Load calculation error: deltaTotal <= 0", LogLevel::error);
        }
    } else {
        logger.saveMessage(" Failed to read CPU stats from /proc/stat", LogLevel::error);
    }
}

void SystemMonitor::monitorMemory(LogLevel userLogLevel) {
    std::ifstream memFile("/proc/meminfo");
    if (!memFile) {
        logger.saveMessage(" Failed to open /proc/meminfo", LogLevel::error);
        return;
    }

    long memTotal     = 0;
    long memAvailable = 0;

    std::string line;
    while (std::getline(memFile, line)) {
        if (line.find("MemTotal:") == 0) {
            // Извлекаем значение MemTotal
            std::istringstream iss(line);
            std::string        label;
            iss >> label >> memTotal;
        } else if (line.find("MemAvailable:") == 0) {
            // Извлекаем значение MemAvailable
            std::istringstream iss(line);
            std::string        label;
            iss >> label >> memAvailable;
        }

        // Если оба значения найдены, можно остановить чтение
        if (memTotal > 0 && memAvailable > 0) {
            break;
        }
    }

    // Если данные успешно извлечены
    if (memTotal > 0 && memAvailable > 0) {
        long   usedMemory   = memTotal - memAvailable;
        double totalGB      = static_cast<double>(memTotal) / 1048576.0;    // Перевод в гигабайты
        double usedGB       = static_cast<double>(usedMemory) / 1048576.0;  // Перевод в гигабайты
        double usagePercent = (usedGB / totalGB) * 100.0;                   // Процент использования

        getLoadBoundary(userLogLevel);
        if (loadmin <= usagePercent && usagePercent <= loadmax) {
            std::ostringstream message;
            message << "Memory Usage: " << usedGB << " GB used of " << totalGB << " GB total (" << usagePercent << "%)";
            writeToOutputFile(message.str()); // Сохраняем в файл
        }

        // Формируем понятное сообщение для пользователя
        std::ostringstream message;
        message << " Memory Usage: " << usedGB << " GB used of " << totalGB << " GB total (" << usagePercent << "%)";

        logger.saveMessage(message.str(), getLevelfromBound(usagePercent));
    } else {
        logger.saveMessage(" Failed to parse memory info", LogLevel::error);
    }
}

void SystemMonitor::monitorDisk(LogLevel userLogLevel) {
    struct statvfs fs;
    if (statvfs("/", &fs) != 0) {
        logger.saveMessage(" Failed to get disk stats", LogLevel::error);
        return;
    }

    // Общая емкость и свободное место в гигабайтах
    double totalGB = static_cast<double>(fs.f_blocks) * fs.f_frsize / (1024 * 1024 * 1024);
    double freeGB  = static_cast<double>(fs.f_bfree) * fs.f_frsize / (1024 * 1024 * 1024);
    double usedGB  = totalGB - freeGB;

    // Процент использования
    double usedPercent = (usedGB / totalGB) * 100.0;

    getLoadBoundary(userLogLevel);
    if (loadmin <= usedPercent && usedPercent <= loadmax) {
        std::ostringstream oss;
        oss << "Disk usage for root filesystem: Total space = " << totalGB << " GB, Used = " << usedGB << " GB (" 
            << usedPercent << "%)";
        writeToOutputFile(oss.str()); // Сохраняем в файл
    }

    // Формирование читаемого сообщения
    std::ostringstream oss;
    oss << " Disk usage for root filesystem: Total space = " << totalGB << " GB, Used = " << usedGB << " GB ("
        << usedPercent << "%)";

    logger.saveMessage(oss.str(), getLevelfromBound(usedPercent));
}

void SystemMonitor::getLoadBoundary(LogLevel userLogLevel) {
    if (userLogLevel == LogLevel::info) {
        loadmin = 0;
        loadmax = 50;
    } else if (userLogLevel == LogLevel::warning) {
        loadmin = 51;
        loadmax = 80;
    } else if (userLogLevel == LogLevel::error) {
        loadmin = 81;
        loadmax = 100;
    }
}
LogLevel SystemMonitor::getLevelfromBound(int persant){
    if (persant <= 50){
        return LogLevel::info;
    }
    else if (persant <= 80){
        return LogLevel::warning;
    }
    else{
        return LogLevel::error;
    }
}



// int main() {
//     Logger        logger("kfjdkl.txt", "info");
//     SystemMonitor l1(logger);
//     l1.monitorCPU(LogLevel::info);
// }