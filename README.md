# Библиотека для записи сообщений в журнал (Logger)

Этот проект представляет собой библиотеку для записи сообщений в журнал (лог-файл) с различными уровнями важности, а также консольное приложение, демонстрирующее её работу.

## Описание

Библиотека `Logger` предоставляет функциональность для записи текстовых сообщений в журнал, хранящийся в текстовом файле. Она позволяет задавать уровень важности сообщений и записывать в лог-файл информацию о времени получения сообщения. Консольное приложение демонстрирует использование библиотеки в многопоточном режиме, позволяя пользователю вводить сообщения и их уровни важности.

## Часть 1: Библиотека `Logger`

### Требования

- Библиотека является динамической.
- При инициализации принимает:
  - Имя файла журнала.
  - Уровень важности сообщения по умолчанию (используется перечисление с уровнями `info`, `warning`, `error`).
- В журнале сохраняется:
  - Время получения сообщения.
  - Уровень важности.
  - Текст сообщения.
- После инициализации можно изменить уровень важности по умолчанию.

### Использование библиотеки

```c++
#include <logger/logger.h>

int main() {
  Logger logger("my_log.txt", LogLevel::info);

  logger.log("Hello world!", LogLevel::warning);
  logger.log("Can't", LogLevel::info); // не запишется!
  logger.log("Err...", LogLevel::error);

  logger.changeLogLevel(LogLevel::info); // меняем уровень по умолчанию
  logger.log("Bye!", LogLevel::info);

  return 0;
}
```

### Формат записи в журнале

Каждая запись в журнале имеет следующий формат:

`[YYYY-MM-DD hh:mm:ss][УРОВЕНЬ] Сообщение`

Пример:

_[2025-01-24 16:41:58][INFO] Average CPU Load: 1.15%_

## Часть 2: Консольное приложение

Требования

- Подключает и использует библиотеку Logger.
- Принимает от пользователя в консоли сообщение и его уровень важности (уровень может отсутствовать, тогда используется уровень по умолчанию).
- Передает данные в отдельный поток для записи в журнал (потокобезопасно).
- Ожидает новый ввод от пользователя после передачи данных.
- Принимает параметры: имя файла журнала и уровень важности по умолчанию.

### Запуск приложения

```bash
./app <имя_файла_журнала> <уровень_важности>
```

Где:

<имя*файла*журнала> - имя файла, в который будут записываться сообщения. Программа автоматически сделает его с расширением txt.
<уровень_важности> - уровень важности по умолчанию (info, warning, error).

Пример:

```bash
./app app_logs info
...
```

Приложение ожидает ввода в формате:

<сообщение>

[<уровень_важности>] - может быть пустым, если нажать 'Enter'

# Сборка проекта

Для начала требуется установить библиотеку в систему:

```bash
make install
```

Исполнить все цели:

```bash
make
```

Очистить мусор:

```bash
make clean
```

Удалить библиотеку из системы:

```bash
make uninstall
```

После успешной сборки с целью all в директории `build` будут созданы:

- `liblogger.so` - динамическая библиотека.
- `app` - исполняемый файл приложения.
- `test` - исполняемый файл тестов.

# Структура проекта

```
.
├── src # исходный код
│ ├── logger # библиотека
│ ├── monitoring # отслеживание системных компонентов
| ├── multithreading # всё для работы с многопоточностью
│ └── main.cpp # точка входа в приложение
├── tests # тесты
├── .clang-format
├── .gitignore
└── README.md
```

# Зависимости

Проект не имеет внешних зависимостей, кроме стандартной библиотеки C++.
