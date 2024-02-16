/* utils.h: Utility Macros */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

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

/* bit conversion (BIG Endian) */
// the get functions get an integer from a byte array
uint32_t get_uint32(uint8_t buf[4]);

uint64_t get_uint64(uint8_t buf[8]);

// the write function convert the integer n to a byte array and write it in the tab argument
void write_uint64_arr(uint8_t tab[8], uint64_t n);

void write_uint32_arr(uint8_t tab[4], uint32_t n);
#endif

