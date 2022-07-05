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

    // Create an attribute mask
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK| BRKINT| PARMRK| ISTRIP| INLCR| IGNCR| ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;
    
    if (tcsetattr(devfile, TCSANOW, &tty) != 0) {
        perror("Failed to set attributes");
        return -1;
    }

    return devfile;
}


int read_one(int fd, char* buf) {
retry:
    // Try to read a single byte
    switch (read(fd, buf, 1)) {
        case 1: return 0;
        case 0: goto retry;
        default:
            perror("Failed to read byte");
            return -1;
    }
}


int write_one(int fd, const char* byte) {
    // Write a single byte
    switch (write(fd, byte, 1)) {
        case 1: break;
        case 0: errno = EOF;
        default:
            perror("Failed to read byte");
            return -1;
    }

    // Flush output
    if (fsync(fd) != 0) {
        perror("Failed to write data");
        return -1;
    }

    return 0;
}
