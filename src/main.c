#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include "io.h"


/**
 * @brief Displays the help and exists
 */
void help() {
    // Print the errno
    if (errno != 0) {
        perror("Fatal error");
    }

    // Print the help
    printf("Usage: spipe <devicefile> <baudrate>\n");
    printf("\n");
    exit(errno);
}


/**
 * @brief Handles ctrl+c
 * 
 * @param signum The signal number
 */
void ctrlc(int signum) {
    // Drop arguments and define message
    (void)signum;
    const char message[] = "Received interrupt - stopping...\n";

    // Write an info message and terminate
    write(STDERR_FILENO, "Received interrupt\n", sizeof(message));
    exit(EINTR);
}


/**
 * @brief An argument for `copy_loop`
 */
struct copy_loop_args {
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
    char buf;

    // Loop
    while (1) {
        // Read from stdin
        if (read_one(args->src, &buf) != 0) {
            perror("Failed to read from source");
            return NULL;
        }

        // Write to stdout
        if (write_one(args->dest, &buf) != 0) {
            perror("Failed to write to destination");
            return NULL;
        }
    }
    return NULL;
}


int main(int argc, char** argv) {
    // Register signal handlers
    signal(SIGINT, ctrlc);
    signal(SIGTERM, ctrlc);

    // Validate the arguments
    if (argc < 3) {
        errno = EINVAL;
        help();
    }

    // Parse the baudrate
    unsigned long baudrate;
    if (parse_long(argv[2], &baudrate) != 0) {
        perror("Invalid baudrate");
        exit(errno);
    }

    // Open the serial file
    int devfile = open_serial(argv[1], baudrate);
    if (devfile < 0) {
        perror("Failed to open device file");
        exit(errno);
    }

    // Start I/O
    pthread_t to_serial, from_serial;
    struct copy_loop_args to_serial_args = { .src = STDIN_FILENO, .dest = devfile };
    struct copy_loop_args from_serial_args = { .src = devfile, .dest = STDOUT_FILENO };

    if ((errno = pthread_create(&to_serial, NULL, copy_loop, &to_serial_args)) != 0) {
        perror("Failed to spawn stdin->serial thread");
        exit(errno);
    }
    if ((errno = pthread_create(&from_serial, NULL, copy_loop, &from_serial_args)) != 0) {
        perror("Failed to spawn serial->stdout thread");
        exit(errno);
    }

    // Wait until the threads exit
    pthread_join(to_serial, NULL);
    pthread_join(from_serial, NULL);
    return errno;
}
