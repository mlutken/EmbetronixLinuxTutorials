#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

void open_and_wait(const char* file_path)
{
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open sysfs attribute");
        return;
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

//    while (1) {
        // Use select to wait for changes in the sysfs attribute
        int result = select(fd + 1, &read_fds, NULL, NULL, NULL);
        if (result == -1) {
            perror("select");
            close(fd);
            return;
        }

        if (FD_ISSET(fd, &read_fds)) {
            // Sysfs attribute has changed, read and print the new value
            char buffer[10];  // Assuming a reasonable buffer size
            ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
            if (bytesRead == -1) {
                perror("read");
                close(fd);
                return;
            }

            buffer[bytesRead] = '\0';
            printf("Received update: %s", buffer);
        }

        // Reset file descriptor set for the next iteration
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
  //  }

    close(fd);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file_path = argv[1];
    for (int i = 3; i > 0; --i) {
        open_and_wait(file_path);
    }
    return 0;
}


