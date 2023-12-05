/* utils.h: Utility Macros */

#ifndef UTILS_H
#define UTILS_H

/* Macros */

#define min(a, b)   \
    (((a) < (b)) ? (a) : (b))

#define max(a, b)   \
    (((a) > (b)) ? (a) : (b))

/* Logging Prototypes */

void enable_logging_to_file();

void disable_logging_to_file();

void log_error(const char *fmt, ...);

void log_info(const char *fmt, ...);

void log_debug(const char *fmt, ...);

void log_warning(const char *fmt, ...);

void close_log_file(); /* TODO: Temporary */

#endif

