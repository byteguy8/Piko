#include "error_report.h"

void report_error(int code, char *location, const char *err, va_list arg)
{
    vfprintf(stderr, err, arg);
    printf("\nat: %s\n", location);

    memory_deinit();

    exit(code);
}

void report_error_at(int code, int line, char *location, const char *err, va_list arg)
{
    printf("ERROR (from: %s) at line %d:\n", location, line);
    vfprintf(stderr, err, arg);
    printf("\n");

    memory_deinit();

    exit(code);
}