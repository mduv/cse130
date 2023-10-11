#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#define MAX_INPUTLINE_SIZE 4096
#define MAX_BUFFER_SIZE 4096

void get(const char *location) {
    // if (location[strlen(location)] != '\n') {
    //     write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
    //     exit(1);
    // }
    int fd = open(location, O_RDONLY);
    if (fd == -1) {
        write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
        exit(1);
    }

    char buf[MAX_BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = read(fd, buf, MAX_BUFFER_SIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, bytesRead) == -1) {
            write(STDERR_FILENO, "Invalid Command: unable to write\n", sizeof("Invalid Command: unable to write\n") - 1);
            exit(1);
        }
    }

    if (bytesRead == -1) {
        write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
        exit(1);
    }

    close(fd);
}



void set(const char *location, int length, const char *content) {
    int fd = open(location, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        write(STDERR_FILENO, "Invalid Command: Unable to open set\n", sizeof("Invalid Command: Unable to open set\n") - 1);
        exit(1);
    }

    if (content == NULL) {
        printf("OK\n");
        exit(0);
    }

    ssize_t bytesWritten = write(fd, content, length - 1);
    // printf("Set - bytesWritten: %zd\n", bytesWritten);

    if (bytesWritten == -1) {
        write(STDERR_FILENO, "Invalid Command: error bytes written\n", sizeof("Invalid Command: error bytes written\n") - 1);
        close(fd);
        exit(1);
    }

    // Append a newline character
    if (write(fd, "\n", 1) == -1) {
        perror("Error writing newline to file");
        close(fd);
        exit(1);
    }

    // printf("Set - Content being written: %d, %s\n", length, content);

    

    close(fd);
    printf("OK\n");
    exit(0);
}


int main() {
    char inputLine[MAX_INPUTLINE_SIZE];
    // printf("Main - inputLine: %s\n", inputLine);
    int bytesRead = read(0, inputLine, MAX_INPUTLINE_SIZE);
    // printf("Main - bytesRead: %d\n", bytesRead);

    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            write(STDERR_FILENO, "Invalid command: stdin was closed before user provided a location\n", sizeof("Invalid command: stdin was closed before user provided a location\n") - 1);
        } else {
            // printf("error reading command\n");
        }
        exit(1);
    }
   

    // Check if there is a newline character at the end
    // if (inputLine[bytesRead - 1] != '\n') {
    //     write(STDERR_FILENO, "Invalid command: Missing newline character at the end\n", sizeof("Invalid command: Missing newline character at the end\n") - 1);
    //     exit(1);
    // }

    // Ensure null-termination of the input line
    // inputLine[bytesRead] = '\0';

    // Parse the command and handle get or set
    char *token = strtok(inputLine, "\n");
    // printf("Main - token: %s\n", token);
    if (token != NULL) {
        if (strcmp(token, "get") == 0) {
            // Handle get command
            token = strtok(NULL, "\n");
            if (token == NULL) {
                write(STDERR_FILENO, "Invalid Command: No location provided after 'get'\n", sizeof("Invalid Command: No location provided after 'get'\n") - 1);
                exit(1);
            }
            token = strtok(NULL, "\n");
            if (token != NULL) {
                write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
                exit(1);
            }
            get(token);
        } else if (strcmp(token, "set") == 0) {
            // Handle set command
            char* location = strtok(NULL, "\n");
            // printf("Main - location: %s\n", location);
            if (location == NULL) {
                write(STDERR_FILENO, "Invalid Command: No location provided after 'set'\n", sizeof("Invalid Command: No location provided after 'set'\n") - 1);
                exit(1);
            }
            

            // Read the content length and content
            token = strtok(NULL, "\n");
            if (token == NULL) {
                write(STDERR_FILENO, "Invalid Command: Missing content length after 'set'\n", sizeof("Invalid Command: Missing content length after 'set'\n") - 1);
                exit(1);
            }
            // printf("Main - token: %s\n", token);
            int contentLength = atoi(token);


            // Check if the last character is a newline
            if (contentLength < 0) {
                write(STDERR_FILENO, "Invalid Command: Negative content length\n", sizeof("Invalid Command: Negative content length\n") - 1);
                exit(1);
            }

            char* content;
            content = strtok(NULL, "\n");

            
            // printf("Main - content: %s\n", content);

            // if (content[bytesRead - 1] == '\n') {
            //     printf("Last character is a newline character.\n");
            // } else {
            //     printf("Last character is not a newline character.\n");
            // }

            // printf("Main - content length: %d\n", contentLength);   
            // Process the set command with the valid content
            set(location, contentLength, content);
        } else {
            write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
            exit(1);
        }
    } else {
        write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
        exit(1);
    }

    return 0;
}


