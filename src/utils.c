#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ERROR_MSG "\033[1;31mERROR:\033[0m "
#define WARNING_MSG "\033[1;33mWARNING:\033[0m "
#define INFO_MSG "\033[1;34mINFO:\033[0m "
#define DEBUG_MSG "\033[1;35mDEBUG:\033[0m "

#define LOG_PREFIX "log_"
#define LOG_SUFFIX ".log"

int should_log_to_file = 0;

FILE *create_log_file();
FILE *log_file;

void close_log_file()
{
 if (should_log_to_file)
		fclose(log_file);
}

void enable_logging_to_file()
{
 if (should_log_to_file)
		return;

 should_log_to_file = 1;
 log_file = create_log_file();
}

void disable_logging_to_file()
{
 should_log_to_file = 0;
}

void log_error(const char *fmt, ...)
{

 va_list args;
 va_start(args, fmt);

 fprintf(stderr, ERROR_MSG);
 vfprintf(stderr, fmt, args);

 if (should_log_to_file)
 {
		fprintf(log_file, ERROR_MSG);
		vfprintf(log_file, fmt, args);
 }

 va_end(args);
}

void log_warning(const char *fmt, ...)
{
 va_list args;
 va_start(args, fmt);
 fprintf(stdout, WARNING_MSG);
 vfprintf(stdout, fmt, args);

 if (should_log_to_file)
 {
		fprintf(log_file, WARNING_MSG);
		vfprintf(log_file, fmt, args);
 }

 va_end(args);
}

void log_info(const char *fmt, ...)
{
 va_list args;
 va_start(args, fmt);
 fprintf(stdout, INFO_MSG);
 vfprintf(stdout, fmt, args);

 if (should_log_to_file)
 {
		fprintf(log_file, INFO_MSG);
		vfprintf(log_file, fmt, args);
 }

 va_end(args);
}

void log_debug(const char *fmt, ...)
{
 va_list args;
 va_start(args, fmt);
 fprintf(stdout, DEBUG_MSG);
 vfprintf(stdout, fmt, args);

 if (should_log_to_file)
 {
		fprintf(log_file, DEBUG_MSG);
		vfprintf(log_file, fmt, args);
 }

 va_end(args);
}

FILE *create_log_file()
{
 FILE *log_file = fopen("log.txt", "w");

 if (log_file == NULL)
 {
		fprintf(stderr, "Error opening log file\n");
		exit(1);
 }

 return log_file;
}

uint32_t get_uint32(uint8_t buf[4]) {
 return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

uint64_t get_uint64(uint8_t buf[8]) {
 return ((uint64_t)buf[0] << 56) | ((uint64_t)buf[1] << 48) | ((uint64_t)buf[2] << 40) | ((uint64_t)buf[3] << 32) | ((uint64_t)buf[4] << 24) | ((uint64_t)buf[5] << 16) | ((uint64_t)buf[6] << 8) | buf[7];
}

void write_uint64_arr(uint8_t tab[8], uint64_t n) {
	unsigned int decal = 56;
	
	for (int i = 0; i < 8; i++) {
		tab[i] = n >> decal;
		decal -= 8;
	}
}

void write_uint32_arr(uint8_t tab[4], uint32_t n) {
	unsigned int decal = 24;
	
	for (int i = 0; i < 4; i++) {
		tab[i] = n >> decal;
		decal -= 8;
	}
}
