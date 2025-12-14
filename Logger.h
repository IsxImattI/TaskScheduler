#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>
#include <iostream>

// log levels
enum LogLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_SUCCESS,
    LOG_TASK
};

class Logger {
private:
    CRITICAL_SECTION cs;
    HANDLE hConsole;
    
    // get current timestamp
    void getTimestamp(char* buffer, size_t bufferSize) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        // format: [HH:MM:SS.mmm]
        sprintf_s(buffer, bufferSize, "[%02d:%02d:%02d.%03d]", 
                  st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    }
    
    // set console color
    void setColor(LogLevel level) {
        WORD color;
        
        switch(level) {
            case LOG_INFO:
                color = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                break;
            case LOG_WARNING:
                color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case LOG_ERROR:
                color = FOREGROUND_RED | FOREGROUND_INTENSITY;
                break;
            case LOG_SUCCESS:
                color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case LOG_TASK:
                color = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                break;
            default:
                color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
        
        SetConsoleTextAttribute(hConsole, color);
    }
    
    // reset color
    void resetColor() {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    
    // get log level string
    const char* getLevelString(LogLevel level) {
        switch(level) {
            case LOG_INFO:    return "[INFO]   ";
            case LOG_WARNING: return "[WARNING]";
            case LOG_ERROR:   return "[ERROR]  ";
            case LOG_SUCCESS: return "[SUCCESS]";
            case LOG_TASK:    return "[TASK]   ";
            default:          return "[UNKNOWN]";
        }
    }
    
public:
    Logger() {
        InitializeCriticalSection(&cs);
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    
    ~Logger() {
        DeleteCriticalSection(&cs);
    }
    
    // log message with level
    void log(LogLevel level, const char* message) {
        EnterCriticalSection(&cs);
        
        char timestamp[32];
        getTimestamp(timestamp, sizeof(timestamp));
        
        // print timestamp (white)
        resetColor();
        std::cout << timestamp << " ";
        
        // print level (colored)
        setColor(level);
        std::cout << getLevelString(level);
        
        // print message (white)
        resetColor();
        std::cout << " " << message << std::endl;
        
        LeaveCriticalSection(&cs);
    }
    
    // convenience methods
    void info(const char* message) {
        log(LOG_INFO, message);
    }
    
    void warning(const char* message) {
        log(LOG_WARNING, message);
    }
    
    void error(const char* message) {
        log(LOG_ERROR, message);
    }
    
    void success(const char* message) {
        log(LOG_SUCCESS, message);
    }
    
    void task(const char* message) {
        log(LOG_TASK, message);
    }
    
    // formatted logging
    void logf(LogLevel level, const char* format, ...) {
        char buffer[512];
        
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        log(level, buffer);
    }
};

// global logger instance
static Logger globalLogger;

#endif