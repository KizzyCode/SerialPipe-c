#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>


int parse_long(const char* str, unsigned long* buf) {
    // Parse string
    char* strend = NULL;
    *buf = strtoul(str, &strend, 10);
    if (strlen(strend) > 0) {
        errno = EINVAL;
        perror("Invalid number literal");
        return -1;
    }

    return 0;
}


int open_serial(const char* path, unsigned long bauds) {
    // Open the device file nonblocking
    int devfile = open(path, O_RDWR | O_NONBLOCK);
    if (devfile < 0) {
        perror("Failed to open device file");
        return -1;
    }

    // Make the file blocking again
    int flags = fcntl(devfile, F_GETFL, 0);
    if (fcntl(devfile, F_SETFL, flags & ~O_NONBLOCK) != 0) {
        perror("Failed to make serial blocking");
        return -1;
    }

    // Get the device attributes
    struct termios tty;
    if (tcgetattr(devfile, &tty) != 0) {
        perror("Failed to get serial attributes");
        return -1;
    }

    // Set the speed
    if (cfsetispeed(&tty, bauds) != 0) {
        perror("Failed to set input baudrate");
        return -1;
    }
    if (cfsetospeed(&tty, bauds) != 0) {
        perror("Failed to set output baudrate");
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
        perror("Failed to set attributes");
        return -1;
    }

    return devfile;
}


int read_one(int fd, char* buf) {
retry:
    {
        // Try to read a single byte
        ssize_t read_ = read(fd, buf, 1);
        if (read_ == 0) {
            goto retry;
        }
        if (read_ < 1) {
            perror("Failed to read byte");
            return -1;
        }

        return 0;
    }
}


int write_one(int fd, const char* byte) {
    // Write a single byte
    ssize_t written = write(fd, byte, 1);
    if (written == 0) {
        errno = EOF;
    }
    if (written < 1) {
        perror("Failed to write byte");
        return -1;
    }

    // Flush output
    if (fsync(fd) != 0) {
        perror("Failed to write data");
        return -1;
    }

    return 0;
}
