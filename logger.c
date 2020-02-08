#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "logger.h"

int logLevel = LEVEL_TRACE;

/**
 * Returns the log level as a representative string
 * @param level the log level
 * @returns the string representation
 */
char* getLogLevelString(int level) {
    switch(level) {
        case LEVEL_TRACE: return "TRACE";
        case LEVEL_DEBUG: return "DEBUG";
        case LEVEL_INFO: return "INFO";
        case LEVEL_WARNING: return "WARNING";
        case LEVEL_ERROR: return "ERROR";
        default: return "-----";
    }
}

/**
 * Adds a statement to the log
 * @param level the log level for this message
 * @param the format of the log message
 * @param ... parameters for the message
 */
void log(const int level, const char* format, ...) {
    if(level < logLevel) {
        return;
    }

    FILE* stream = (level >= LEVEL_WARNING) ? stderr : stdout;

    time_t now;
    time(&now);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", gmtime(&now));
    fprintf(stream, "%s %s -- ", buffer, getLogLevelString(level));

    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);

    fputc('\n', stream);
}

/**
 * Set the current log level
 * @param level the new log level
 */
void setLogLevel(int level) {
    logLevel = level;
}