#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <inttypes.h>
#include "io.h"


/**
 * @brief Displays the help and exists
 */
void help(void) {
    // Print the errno
    if (errno != 0) {
        perror("fatal error");
    }

    // Print the help
    fprintf(stderr, "Usage: spipe <devicefile> <baudrate>\n");
    fprintf(stderr, "\n");
    exit(errno);
}


/**
 * @brief Handles ctrl+c
 * 
 * @param signum The signal number
 */
void ctrl_c(int signum) {
    // Drop arguments and define message
    (void)signum;
    const char message[] = "stopping due to interrupt...\n";

    // Write an info message as best-effort approach and terminate
    ssize_t written = write(STDERR_FILENO, message, sizeof(message));
    (void)written;
    exit(EINTR);
}


/**
 * @brief Parses a string into an integer
 * 
 * @param buf The target buffer
 * @param str The string to parse
 * @return `0` or `-1` in case of an error
 */
int parse_int(int* buf, const char* str) {
    // Parse string
    errno = 0;
    char* strend = NULL;
    *buf = strtoumax(str, &strend, 10);
    
    // Check for errors
    if (errno != 0) {
        return -1;
    }
    if (strlen(strend) > 0) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}


/**
 * @brief An argument for `copy_loop`
 */
struct copy_loop_args {
    /**
     * @brief The name of the copy loop
     */
    const char* name;
    /**
     * @brief The source file descriptor
     */
    int src;
    /**
     * @brief The destination file descriptor
     */
    int dest;
};
/**
 * @brief The stdin->serial copy thread
 * @param devfile_ptr An `int`-pointer to the serial device file descriptor
 * @return `NULL`
 */
void* copy_loop(void* args_ptr) {
    // Get the dev file and create buffer
    struct copy_loop_args* args = (struct copy_loop_args*)args_ptr;
    iobuf_t buf = { 0 };

    // Loop
    while (1) {
        // Read from source
        if (fill_buf(&buf, args->src) != 0) {
            fprintf(stderr, "broken pipe for %s: %s\n", args->name, strerror(errno));
            exit(errno);
        }

        // Write to dest
        if (flush_buf(args->dest, &buf) != 0) {
            fprintf(stderr, "broken pipe for %s: %s\n", args->name, strerror(errno));
            exit(errno);
        }
    }
}


int main(int argc, char** argv) {
    // Register signal handlers
    signal(SIGINT, ctrl_c);
    signal(SIGTERM, ctrl_c);
    signal(SIGPIPE, SIG_IGN);

    // Validate the arguments
    if (argc < 3) {
        errno = EINVAL;
        help();
    }

    // Parse the baudrate
    int baudrate;
    if (parse_int(&baudrate, argv[2]) != 0) {
        perror("invalid baudrate");
        exit(errno);
    }

    // Open the serial file
    int devfile;
    if (open_serial(&devfile, argv[1], (uint32_t)baudrate) < 0) {
        perror("failed to open device file");
        exit(errno);
    }

    // Start I/O
    pthread_t to_serial, from_serial;
    struct copy_loop_args to_serial_args = { .name = "stdin->serial", .src = STDIN_FILENO, .dest = devfile };
    struct copy_loop_args from_serial_args = { .name = "serial->stdout", .src = devfile, .dest = STDOUT_FILENO };

    if ((errno = pthread_create(&to_serial, NULL, copy_loop, &to_serial_args)) != 0) {
        perror("failed to spawn stdin->serial thread");
        exit(errno);
    }
    if ((errno = pthread_create(&from_serial, NULL, copy_loop, &from_serial_args)) != 0) {
        perror("failed to spawn serial->stdout thread");
        exit(errno);
    }

    // Wait until the threads exit
    pthread_join(to_serial, NULL);
    pthread_join(from_serial, NULL);
}
