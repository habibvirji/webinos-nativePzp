#ifndef LOGGER_H
#define LOGGER_H

void logPrint(char *file, int line, char *fmt, ...);
#define LOG(...) (logPrint(__FILE__, __LINE__, __VA_ARGS__))

#endif