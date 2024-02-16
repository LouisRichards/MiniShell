/* utils.h: Utility Macros */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>

/* Macros */

#define BIT_MASK(bit)             (1 << (bit))
#define BIT_SET(value,bit)        ((value) |= BIT_MASK(bit))
#define BIT_CLEAR(value,bit)      ((value) &= ~BIT_MASK(bit))
#define BIT_TEST(value,bit)       (((value) & BIT_MASK(bit)) ? 1 : 0)

/* Logging Prototypes */

void enable_logging_to_file();

void disable_logging_to_file();

void log_error(const char *fmt, ...);

void log_info(const char *fmt, ...);

void log_debug(const char *fmt, ...);

void log_warning(const char *fmt, ...);

void close_log_file(); /* TODO: Temporary */

/* bit conversion (BIG Endian) */
uint8_t bit_arr_to_uint8(bool bits[8]);

// the get functions get an integer from a byte array
uint32_t get_uint32(uint8_t buf[4]);

uint64_t get_uint64(uint8_t buf[8]);

// the write function convert the integer n to a byte array and write it in the tab argument
void write_uint64_arr(uint8_t tab[8], uint64_t n);

void write_uint32_arr(uint8_t tab[4], uint32_t n);

int math_ceil(double num);
#endif

