#ifndef IO_H
#define IO_H

#include <unistd.h>


/**
 * @brief Parses a string into a long
 * 
 * @param str The string to parse
 * @param buf The target buffer
 * @return `0` or `-1` in case of an error
 */
int parse_long(const char* str, unsigned long* buf);


/**
 * @brief Opens a serial device file
 * 
 * @param path The path to open
 * @param bauds The baud rate to configure
 * @return The device file descriptor or `-1` in case of an error
 */
int open_serial(const char* path, unsigned long bauds);


/**
 * @brief Reads one byte from `fd`
 * 
 * @param fd The file descriptor to write to
 * @param buf The target buffer
 * @return `0` or `-1` on error
 */
int read_one(int fd, char* buf);


/**
 * @brief Writes one byte to `fd`
 * 
 * @param fd The file descriptor to write to
 * @param byte The byte to write
 * @return `0` or `-1` on error
 */
int write_one(int fd, const char* byte);


#endif // IO_H
