#include <logger/logger.h>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "../src/monitoring/monitoring.h"
#include "../src/multithreading/multithreading.h"

// Проверка выброса исключения при недоступном файле
void testLoggerFailsToOpenRestrictedFile() {
    const std::string invalidFile = "restricted_test_log.txt";

    // Создаем файл и ограничиваем доступ
    {
        std::ofstream file(invalidFile);
        file << "Test content";  // Записываем что-то в файл
    }
    std::filesystem::permissions(invalidFile, std::filesystem::perms::none);  // Убираем все права доступа

    try {
        // Попытка создать Logger для недоступного файла должна выбросить исключение
        Logger logger(invalidFile, "info");

        // Если мы дошли до сюда, то исключение не выброшено, тест провален
        assert(false && "Logger did not throw exception for restricted file");
    } catch (const std::runtime_error& e) {
        // Проверяем, что выброшено ожидаемое исключение с корректным сообщением
        std::string expectedError = "Unable to open log file: " + invalidFile;
        assert(std::string(e.what()) == expectedError && "Exception message does not match expected error");

        // Если исключение поймано с корректным сообщением, тест пройден
        std::cout << "testLoggerFailsToOpenRestrictedFile passed\n";
    } catch (...) {
        // Если выброшено неожиданное исключение
        assert(false && "Unexpected exception type");
    }

    // Восстанавливаем доступ и удаляем файл
    std::filesystem::permissions(invalidFile, std::filesystem::perms::all);  // Восстанавливаем права
    std::filesystem::remove(invalidFile);                                    // Удаляем файл
}

// Проверка записи сообщения
void testLoggerWritesMessage() {
    const std::string logFile = "test_log.txt";
    Logger            logger(logFile, "info");

    // Пишем сообщение в лог
    logger.saveMessage("Test message", LogLevel::info);

    // Читаем содержимое лога
    std::ifstream file(logFile);
    std::string   content;
    std::getline(file, content);

    // Проверяем, что сообщение записано
    assert(content.find("Test message") != std::string::npos && "Message was not written to log");

    // Если assert не сработал, выводим успешное сообщение
    std::cout << "testLoggerWritesMessage passed\n";

    // Удаляем тестовый файл
    std::filesystem::remove(logFile);
}

// Проверка уровня логирования
void testLoggerRespectsLogLevel() {
    const std::string logFile = "test_log.txt";
    Logger            logger(logFile, "warning");

    logger.saveMessage("This is info", LogLevel::info);        // Не должно записаться
    logger.saveMessage("This is warning", LogLevel::warning);  // Должно записаться

    std::ifstream file(logFile);
    std::string   content;
    std::getline(file, content);

    assert(content.find("This is warning") != std::string::npos && "Warning level message not found");
    assert(content.find("This is info") == std::string::npos && "Info level message should not be logged");

    // Если assert не сработал, выводим успешное сообщение
    std::cout << "testLoggerRespectsLogLevel passed\n";
    std::filesystem::remove(logFile);
}

// Проверка смены уровня логирования
void testLoggerChangeLogLevel() {
    const std::string logFile = "test_log.txt";
    Logger            logger(logFile, "info");

    logger.changeLogLevel(LogLevel::error);
    logger.saveMessage("This is info", LogLevel::info);    // Не должно записаться
    logger.saveMessage("This is error", LogLevel::error);  // Должно записаться

    std::ifstream file(logFile);
    std::string   content;
    std::getline(file, content);

    assert(content.find("This is error") != std::string::npos && "Error level message not found");
    assert(content.find("This is info") == std::string::npos && "Info level message should not be logged");

    // Если assert не сработал, выводим успешное сообщение
    std::cout << "testLoggerChangeLogLevel passed\n";
    std::filesystem::remove(logFile);
}

// Проверка записи в многопоточности
void testLoggerMultithreading() {
    const std::string logFile = "test_log.txt";
    Logger            logger(logFile, "info");

    auto logFunction = [&logger](int id) {
        for (int i = 0; i < 10; ++i) {
            logger.saveMessage("Message from thread " + std::to_string(id), LogLevel::info);
        }
    };

    std::thread t1(logFunction, 1);
    std::thread t2(logFunction, 2);

    t1.join();
    t2.join();

    std::ifstream file(logFile);
    int           lineCount = 0;
    std::string   line;
    while (std::getline(file, line)) {
        ++lineCount;
    }

    assert(lineCount == 20 && "Not all messages were logged in multithreaded environment");

    std::cout << "testLoggerMultithreading passed\n";
    std::filesystem::remove(logFile);
}

void testLoggerEmptyFilename() {
    try {
        Logger logger("", "info");
        assert(false && "Logger did not throw exception for empty filename");
    } catch (const std::runtime_error& e) {
        assert(std::string(e.what()) == "Unable to open log file: " && "Exception message is incorrect");
    }
    std::cout << "testLoggerEmptyFilename passed\n";
}

void testLoggerClosesFile() {
    const std::string logFile = "test_log.txt";

    {
        Logger logger(logFile, "info");
        logger.saveMessage("Test message", LogLevel::info);
    }

    std::ofstream file(logFile, std::ios::app);
    assert(file.is_open() && "File was not properly closed by Logger");
    file.close();

    std::cout << "testLoggerClosesFile passed\n";
    std::filesystem::remove(logFile);
}

// Тест на запись большого объема данных
void testLoggerLargeDataVolume() {
    const std::string logFile = "large_data_test_log.txt";
    Logger            logger(logFile, "info");

    // Генерация большого количества сообщений
    const int numMessages = 100000;
    for (int i = 0; i < numMessages; ++i) {
        logger.saveMessage("Message " + std::to_string(i), LogLevel::info);
    }

    // Проверяем количество записанных строк
    std::ifstream file(logFile);
    int           lineCount = 0;
    std::string   line;
    while (std::getline(file, line)) {
        ++lineCount;
    }

    assert(lineCount == numMessages && "Not all messages were logged in large data volume test");
    std::cout << "testLoggerLargeDataVolume passed\n";

    std::filesystem::remove(logFile);
}

void testLoggerInvalidLogLevel() {
    const std::string logFile = "invalid_log_level_test_log.txt";
    Logger            logger(logFile, "unknown");

    try {
        logger.saveMessage("Invalid log level", Logger::translateLevel("some"));
        std::cout << "testLoggerInvalidLogLevel passed (handled gracefully)\n";
    } catch (...) {
        assert(false && "Logger should not throw exception for invalid log level");
    }

    std::ifstream file(logFile);
    std::string   content;
    std::getline(file, content);

    // Проверяем, что сообщение с некорректным уровнем логирования записано как UNKNOWN
    assert(content.find("UNKNOWN") != std::string::npos && "Invalid log level was not logged as UNKNOWN");

    std::filesystem::remove(logFile);
}

std::atomic<bool>        running(true);
std::mutex               threadsMutex;
std::vector<std::thread> monitoringThreads;

void validateAppInput(const std::string& initialLevelStr) {
    if ((Logger::translateLevel(initialLevelStr) != LogLevel::info) &&
        (Logger::translateLevel(initialLevelStr) != LogLevel::warning) &&
        (Logger::translateLevel(initialLevelStr) != LogLevel::error)) {
        std::cout << "You send unknown level\n";
        running = false;
    }
}

// Функция для выполнения мониторинга
void monitoringTask(SystemMonitorManager& systemMonitorManager, LogLevel logLevel) {
    systemMonitorManager.startMonitoring(logLevel);
}

void stopApp(std::thread& input) {
    input.join();
    {
        std::lock_guard<std::mutex> lock(threadsMutex);
        for (auto& thread : monitoringThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
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

void testApplicationInvalidMessage() {
    const std::string initialLevelStr = "info", filename = "logs.txt";
    validateAppInput(initialLevelStr);
    Logger             logger(filename, initialLevelStr);
    std::istringstream inputStream("adsdsaasd");
    std::cin.rdbuf(inputStream.rdbuf());

    std::thread input(inputThread, std::ref(logger));
    stopApp(input);

    std::ifstream file(filename, std::ios::binary);
    assert(file.peek() == std::ifstream::traits_type::eof());
    std::remove(filename.c_str());
}

int main() {
    // library
    testLoggerFailsToOpenRestrictedFile();
    testLoggerWritesMessage();
    testLoggerRespectsLogLevel();
    testLoggerChangeLogLevel();
    testLoggerMultithreading();
    testLoggerEmptyFilename();
    testLoggerClosesFile();
    testLoggerLargeDataVolume();
    testLoggerInvalidLogLevel();

    // application

    testApplicationInvalidMessage();

    std::cout << "All tests passed successfully!\n";
    return 0;
}
