/* utils.h: Utility Macros */

#ifndef UTILS_H
#define UTILS_H

/* Macros */

#define min(a, b)   \
    (((a) < (b)) ? (a) : (b))

#define max(a, b)   \
    (((a) > (b)) ? (a) : (b))

/* Logging Prototypes */

void log_error(const char *fmt, ...);

void log_info(const char *fmt, ...);

void log_debug(const char *fmt, ...);

void log_warning(const char *fmt, ...);

#endif

