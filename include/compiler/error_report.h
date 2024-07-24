#ifndef _ERROR_REPORT_H_
#define _ERROR_REPORT_H_

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void report_error(int code, char *location, const char *err, va_list arg);
void report_error_at(int code, int line, char *location, const char *err, va_list arg);

#endif