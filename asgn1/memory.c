#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/param.h>

#define MAX_INPUTLINE_SIZE 4096
#define MAX_BUFFER_SIZE    4096

char inputLine[MAX_INPUTLINE_SIZE];
int bytesRead;
char original_input[MAX_INPUTLINE_SIZE];

#define DEBUG 0

// function to check if input file given is a directory
int is_directory(int fd) {
    struct stat buf;
    if (fstat(fd, &buf) != 0) {
        return -1;
    }
    if (S_ISDIR(buf.st_mode)) {
        return 1;
    }
    return 0;
}

// reading stdin
int read_input() {
    int bytesRead = 0;
    int totalBytesRead = 0;

    // buffer to read stdin
    while ((bytesRead = read(
                STDIN_FILENO, inputLine + totalBytesRead, MAX_INPUTLINE_SIZE - totalBytesRead))
           > 0) {
        totalBytesRead += bytesRead;

        // Check if buffer is full
        if (totalBytesRead == MAX_INPUTLINE_SIZE) {
            return (totalBytesRead);
        }
    }

    if (bytesRead == -1) {
        fprintf(stderr, "Invalid command: error reading input\n");
        return (-1);
    }
    return totalBytesRead;
}

// write whatever is read into buffer
int write_buffer(int fd, char *buffer, int bytes) {
    int byteToWrite = bytes;
    int byteWritten = 0;
    int totalBytesWritten = 0;

    while ((byteWritten = write(fd, buffer + totalBytesWritten, byteToWrite)) != byteToWrite) {
        if (byteWritten == -1) {
            return (-1);
        }
        totalBytesWritten += byteWritten;
        byteToWrite = byteToWrite - byteWritten;

        fprintf(stderr, "write_buffer: byteswrite %d\n", byteWritten);
    }
    return (bytes);
}

// handle get
void get(const char *location) {

    int fd = open(location, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }

    if (is_directory(fd) != 0) {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }

    char buf[MAX_BUFFER_SIZE];
    int bytesRead;
    int bytesWrite;
    while ((bytesRead = read(fd, buf, MAX_BUFFER_SIZE)) > 0) {
        if ((bytesWrite = write_buffer(STDOUT_FILENO, buf, bytesRead)) == -1) {
            fprintf(stderr, "Operation Failed");
            exit(1);
        }
    }
    close(fd);
}

// handle set
void set(const char *location, int length, const char *content, int content_length) {

    int fd = open(location, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "Operation Failed");
        exit(1);
    }

    if (content == NULL || content_length == 0) {
        printf("OK\n");
        exit(0);
    }

    ssize_t bytesWritten = write(fd, content, MIN(content_length, length));

    int bytesRemaining = length - bytesWritten;

    while ((bytesRead = read_input()) > 0 && (bytesRemaining > 0)) {
        bytesRemaining = bytesRemaining - bytesRead;
        bytesWritten = write(fd, inputLine, bytesRead);

        if (bytesWritten == -1) {
            write(STDERR_FILENO, "Invalid Command: error bytes written\n",
                sizeof("Invalid Command: error bytes written\n") - 1);
            close(fd);
            exit(1);
        }
    }

    if (bytesRead == 0 && bytesRemaining == 1) {
        // we should append a null line as this is striped by strtok()
        write(fd, "\n", 1);
    }

    close(fd);
    printf("OK\n");
    exit(0);
}

int main() {

    int bytesRead = read_input();

    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            fprintf(stderr, "Invalid Command\n");
        } else {
            fprintf(stderr, "Operation Failed");
        }
        exit(1);
    }

    // Ensure null-termination of the input line
    strcpy(original_input, inputLine);
    char *saveptr;
    char *token = strtok_r(inputLine, "\n", &saveptr);
    char *location;
    if (token != NULL) {
        if (strcmp(token, "get") == 0) {
            // Handle get command
            location = strtok_r(NULL, "\n", &saveptr);
            if (location == NULL) {
                fprintf(stderr, "Invalid Command\n");
            }

            if ((bytesRead > 1) && (original_input[bytesRead - 1] != '\n')) {
                fprintf(stderr, "Invalid Command\n");
                exit(1);
            }
            get(location);
        } else if (strcmp(token, "set") == 0) {
            // Handle set command
            char *location = strtok_r(NULL, "\n", &saveptr);
            if (location == NULL) {
                fprintf(stderr, "Invalid Command\n");
                exit(1);
            }

            // Read the content length and content
            token = strtok_r(NULL, "\n", &saveptr);
            if (token == NULL) {
                fprintf(stderr, "Invalid Command\n");
                exit(1);
            }
            int contentLength = atoi(token);

            // Check if the last character is a newline
            if (contentLength < 0) {
                fprintf(stderr, "Invalid Command\n");
                exit(1);
            }

            int bytes_tokenized = saveptr - inputLine;
            int bytes_remaining = bytesRead - bytes_tokenized;

            set(location, contentLength, saveptr, bytes_remaining);
        } else {
            fprintf(stderr, "Invalid Command\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "Invalid command\n");
        exit(1);
    }

    return 0;
}
