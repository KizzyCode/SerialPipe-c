#ifndef IO_H
#define IO_H

#include <unistd.h>
#include <stdint.h>


/**
 * @brief An I/O buffer
 */
typedef struct {
    /**
     * @brief The buffer
     */
    uint8_t data[4096];
    /**
     * @brief The amount of bytes within the buffer
     */
    size_t filled;
} iobuf_t;


/**
 * @brief Opens a serial device file
 * 
 * @param fd The pointer to write the newly created file descriptor into
 * @param path The path to open
 * @param bauds The baud rate to configure
 * @return `0` or `-1` on error 
 */
int8_t open_serial(int* fd, const char* path, uint32_t bauds);


/**
 * @brief Reads as much bytes as possible within a single call; retries if the read was empty
 * 
 * @param buf The target buffer to read into
 * @param fd The file descriptor to read from
 * @return `0` or `-1` on error 
 */
int8_t fill_buf(iobuf_t* buf, int fd);


/**
 * @brief Writes as much bytes as possible within a single call; retries if the write was empty
 * 
 * @param fd The file descriptor to write to
 * @param buf The buffer to write to the file descriptor
 * @return `0` or `-1` on error  
 */
int8_t flush_buf(int fd, iobuf_t* buf);


/**
 * @brief Closes a serial device file descriptor
 * 
 * @param fd The file descriptor to close
 */
void close_serial(int fd);


#endif // IO_H
