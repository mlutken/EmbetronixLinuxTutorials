#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


void open_and_wait(const char* file_path)
{
    int file_descriptor;
    file_descriptor = open(file_path, O_RDONLY);
    if (file_descriptor == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for data on file descriptor %s ...\n", file_path);

    char buffer[1024];

    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_SET(file_descriptor, &read_fds);

    // Set a timeout of xx seconds
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    // Use select to wait for data on the file descriptor
    int result = select(file_descriptor + 1, &read_fds, NULL, NULL, &timeout);

    if (result == -1) {
        perror("select");
        exit(EXIT_FAILURE);
    } else if (result == 0) {
        // Timeout occurred
        printf("Timeout. No data available.\n");
    } else {
        // Data is available for reading
        if (FD_ISSET(file_descriptor, &read_fds)) {
            ssize_t bytesRead = read(file_descriptor, buffer, sizeof(buffer));

            if (bytesRead == -1) {
                perror("read");
                close(file_descriptor);
                exit(EXIT_FAILURE);
            }

            if (bytesRead == 0) {
                printf("End of file reached. Exiting.\n");
            }
            else {
                printf("Read %zd bytes: %.*s\n", bytesRead, (int)bytesRead, buffer);
            }
        }
    }

    // Close the file descriptor
    close(file_descriptor);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file_path = argv[1];
    for (int i = 10; i > 0; --i) {
        open_and_wait(file_path);
    }
    return 0;
}


#if 0
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file_path = argv[1];
    int file_descriptor;

    // Open the specified file
    file_descriptor = open(file_path, O_RDONLY);
    if (file_descriptor == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for data on file descriptor %s...\n", file_path);

    char buffer[1024];

    while (1) {
        fd_set read_fds;
        struct timeval timeout;

        FD_ZERO(&read_fds);
        FD_SET(file_descriptor, &read_fds);

        // Set a timeout of 5 seconds
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        // Use select to wait for data on the file descriptor
        int result = select(file_descriptor + 1, &read_fds, NULL, NULL, &timeout);

        if (result == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (result == 0) {
            // Timeout occurred
            printf("Timeout. No data available.\n");
        } else {
            // Data is available for reading
            if (FD_ISSET(file_descriptor, &read_fds)) {
                ssize_t bytesRead = read(file_descriptor, buffer, sizeof(buffer));

                if (bytesRead == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                }

                if (bytesRead == 0) {
                    printf("End of file reached. Exiting.\n");
                    break;
                }

                printf("Read %zd bytes: %.*s\n", bytesRead, (int)bytesRead, buffer);
            }
        }
    }

    // Close the file descriptor
    close(file_descriptor);

    return 0;
}
#endif

