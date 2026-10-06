# pet_prj_1 (WerFault catcher)
A console utility written in pure **C** for the Windows (Vista+) operating system, designed for process monitoring, tracking application crashes (via intercepting WerFault windows), and maintaining detailed event logs.

## Key Features
- **Process Monitoring:** Scanning active system processes using the Windows API
- **Window Tracking:** Waiting for a target window to appear by its title and resolving its `PID`
- **Safe Logging:** Writing events to a file with preliminary disk space checks and automatic log rotation
- **Flexible Configuration:** Support for command-line arguments (`/debug`, `/sleep`)
- **User-Friendly Interface:** Colored console output for different message types (errors, warnings, timestamps, process statuses) and an animated waiting throbber

## Project Structure
```
pet_prj_1/
¦
+-- main.c              # Main program file (entry point and monitoring logic)
L-- common/
    +-- types.h         # Custom data types and aliases
    +-- functions.h     # Header file for helper functions
    L-- functions.c     # Implementation of helper functions (time, logging, parser)
```
## System Requirements
OS: Windows Vista or newer (requires basic WinAPI support)

Compiler: Any C compiler for Windows (e.g., MinGW-w64 / gcc or MSVC)

## Build and Compilation
If you are using the GCC compiler (part of MinGW) via the command line, build the project with the following command:

    gcc main.c common/functions.c -o werfault_catcher.exe

## Usage
You can run the utility from the command line (cmd or PowerShell) with optional parameters:
Standard mode:

    werfault_catcher.exe

Debug mode:

    werfault_catcher.exe /debug

Set check interval (in milliseconds):

    werfault_catcher.exe /sleep 200

## License
This project is distributed for educational and personal purposes.

---

Консольная утилита на чистом **C** для операционной системы Windows, предназначенная для мониторинга процессов, отслеживания падений приложений (через перехват окон WerFault) и ведения детального лога событий


## Основные возможности
- **Мониторинг процессов:** Сканирование запущенных процессов в системе с использованием Windows API
- **Отслеживание окон:** Ожидание появления целевого окна по его заголовку с определением `PID`
- **Безопасное логирование:** Запись событий в файл с предварительной проверкой свободного места на диске и автоматической ротацией логов
- **Гибкая настройка:** Поддержка аргументов командной строки (`/debug`, `/sleep`).
- **Удобный интерфейс:** Цветной вывод в консоль для разных типов сообщений (ошибки, предупреждения, таймстампы, статусы процессов) и анимированный «троббер» ожидания

## Структура проекта
```
pet_prj_1/
¦
+-- main.c              # Главный файл программы (точка входа и логика мониторинга)
L-- common/
    +-- types.h         # Кастомные типы данных и псевдонимы
    +-- functions.h     # Заголовочный файл вспомогательных функций
    L-- functions.c     # Реализация вспомогательных функций (время, логирование, парсер)
```

## Системные требования
OS: Windows Vista или новее (требуется поддержка базовых функций WinAPI)

Компилятор: Любой компилятор C для Windows (например, MinGW-w64 / gcc или MSVC)

## Сборка и компиляция
Если вы используете компилятор GCC (в составе MinGW) через командную строку, соберите проект следующей командой:

    gcc main.c common/functions.c -o werfault_catcher.exe

## Использование
Запустить утилиту можно из командной строки (cmd или PowerShell) с дополнительными параметрами:
Стандартный режим:

    werfault_catcher.exe

Режим отладки (Debug):

    werfault_catcher.exe /debug

Задать интервал проверки (в миллисекундах):

    werfault_catcher.exe /sleep 200

## Лицензия
Проект распространяется в учебных и личных целях.


