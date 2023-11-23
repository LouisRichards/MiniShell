#include "../include/utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// Path: utils.c

#define ERROR_MSG "\033[1;31mERROR:\033[0m "
#define WARNING_MSG "\033[1;33mWARNING:\033[0m "
#define INFO_MSG "\033[1;34mINFO:\033[0m "
#define DEBUG_MSG "\033[1;35mDEBUG:\033[0m "

void log_error(const char *fmt, ...) 
{
	va_list args;	
	va_start(args, fmt);
	fprintf(stderr, ERROR_MSG);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void log_warning(const char *fmt, ...) 
{
	va_list args;	
	va_start(args, fmt);
	fprintf(stdout, WARNING_MSG);
	vfprintf(stdout, fmt, args);
	va_end(args);
}

void log_info(const char *fmt, ...) 
{
	va_list args;	
	va_start(args, fmt);
	fprintf(stdout, INFO_MSG);
	vfprintf(stdout, fmt, args);
	va_end(args);
}

void log_debug(const char *fmt, ...) 
{
	va_list args;	
	va_start(args, fmt);
	fprintf(stdout, DEBUG_MSG);
	vfprintf(stdout, fmt, args);
	va_end(args);
}
