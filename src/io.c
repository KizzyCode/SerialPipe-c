#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include "io.h"


int parse_long(const char* str, unsigned long* buf) {
    // Parse string
    char* strend = NULL;
    *buf = strtoul(str, &strend, 10);
    if (strlen(strend) > 0) {
        errno = EINVAL;
        perror("invalid number literal");
        return -1;
    }

    return 0;
}


int open_serial(const char* path, uint32_t bauds) {
    // Open the device file nonblocking
    int devfile = open(path, O_RDWR | O_NONBLOCK);
    if (devfile < 0) {
        perror("failed to open device file");
        return -1;
    }

    // Make the file blocking again
    int flags = fcntl(devfile, F_GETFL, 0);
    if (fcntl(devfile, F_SETFL, flags & ~O_NONBLOCK) != 0) {
        perror("failed to make serial blocking");
        return -1;
    }

    // Get the device attributes
    struct termios tty;
    if (tcgetattr(devfile, &tty) != 0) {
        perror("failed to get serial attributes");
        return -1;
    }

    // Set the speed
    if (cfsetspeed(&tty, (speed_t)bauds) != 0) {
        perror("failed to set baudrate");
        return -1;
    }

    // Disable parity generation on output and parity checking for input
    tty.c_cflag &= ~PARENB;
    // Set one stop bit instead of two
    tty.c_cflag &= ~CSTOPB;
    // Use eight bit characters
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    // Disable hardware flow control
    tty.c_cflag &= ~CRTSCTS;
    // Enable receiving
    tty.c_cflag |= CREAD;
    // Ignore modem control lines
    tty.c_cflag |= CLOCAL;
    // Disable canonical mode
    tty.c_lflag &= ~ICANON;
    // Disable INTR, QUIT, SUSP, or DSUSP signals
    tty.c_lflag &= ~ISIG;
    // Disable XON/XOFF
    tty.c_iflag &= ~(IXON | IXOFF);
    // Just allow the START character to restart output
    tty.c_iflag &= ~IXANY;
    // Disable special handling of various signals and parity-errors
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    // Disable implementation-defined output processing
    tty.c_oflag &= ~OPOST;
    // Don't map NL to CR-NL on output
    tty.c_oflag &= ~ONLCR;
    // Minimum number of characters for noncanonical read
    tty.c_cc[VMIN] = 1;
    // Timeout in deciseconds for noncanonical read
    tty.c_cc[VTIME] = 0;
    
    // Apply the updated TTY settings
    if (tcsetattr(devfile, TCSANOW, &tty) != 0) {
        perror("failed to set attributes");
        return -1;
    }

    return devfile;
}


int fill_buf(iobuf_t* buf, int fd) {
    // Compute the available space and return if the buffer full
    const size_t available = sizeof(buf->data) - buf->filled;
    if (available == 0) {
        return 0;
    }

    retry: {
        // Read as much data as possible into the buffer
        ssize_t read_ = read(fd, buf->data + buf->filled, available);
        if (read_ == 0) {
            goto retry;
        }
        if (read_ < 0) {
            perror("failed to read data");
            return -1;
        }

        // Update the buffer
        buf->filled += read_;
        return 0;
    }
}


int flush_buf(int fd, iobuf_t* buf) {
    // Return if the buffer is empty
    if (buf->filled == 0) {
        return 0;
    }

    retry: {
        // Write as much data as possible
        ssize_t written = write(fd, buf->data, buf->filled);
        if (written == 0) {
            goto retry;
        }
        if (written < 0) {
            perror("failed to write data");
            return -1;
        }

        // Sync the file descriptor
        if (fd != STDOUT_FILENO && tcdrain(fd) != 0) {
            perror("failed to sync to serial device");
            return -1;
        }

        // Update the buffer
        buf->filled = buf->filled - written;
        memmove(buf->data, buf->data + written, buf->filled);
        return 0;
    }
}


void close_serial(int fd) {
    close(fd);
}
