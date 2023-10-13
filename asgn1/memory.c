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
#define MAX_BUFFER_SIZE 4096

char inputLine[MAX_INPUTLINE_SIZE];
int bytesRead ;
char original_input[MAX_INPUTLINE_SIZE];


#define DEBUG 0



void debug_printf(char *output, char *fromat) {
    if (DEBUG == 1) {
     printf(output, fromat);
    }
}


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

int read_input() {
    int bytesRead = 0;
    int totalBytesRead = 0;

    // printf("read input\n");

    while ((bytesRead = read(STDIN_FILENO, inputLine + totalBytesRead, MAX_INPUTLINE_SIZE - totalBytesRead)) > 0) {
        totalBytesRead += bytesRead;
        // fprintf(stderr, "total bytes read: %d\n", totalBytesRead);

        // Check if buffer is full
        if (totalBytesRead == MAX_INPUTLINE_SIZE) {
            // fprintf(stderr, "Invalid command: Input buffer full\n");
            return (totalBytesRead);
        }
    }

    if (bytesRead == -1) {
       fprintf(stderr, "Invalid command: error reading input\n");
       return (-1);
    }
    return totalBytesRead;
}

int write_buffer(int fd, char *buffer, int bytes) 
{
        int byteToWrite = bytes;
        int byteWritten = 0;
        int totalBytesWritten = 0;

        while ((byteWritten = write(fd, buffer+totalBytesWritten, byteToWrite)) != byteToWrite) {
            if (byteWritten == -1) {
                return (-1);
            }
            totalBytesWritten += byteWritten;
            byteToWrite = byteToWrite - byteWritten;

            fprintf(stderr, "write_buffer: byteswrite %d\n", byteWritten);

        }
        return (bytes);
}


void get(const char *location) {

    // if ((inputLine[bytesRead-1] != '\n')) {
    //     fprintf(stderr, "Invalid Command\n");
    //     exit(1);
    // }

    int fd = open(location, O_RDONLY);
    // printf("%d\n", fd);
    if (fd == -1) {
        // fprintf(stderr, "Unable to open file for reading\n");
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }

    if (is_directory(fd) != 0)  {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }

    char buf[MAX_BUFFER_SIZE];
    int bytesRead;
    int bytesWrite;
    while ((bytesRead = read(fd, buf, MAX_BUFFER_SIZE)) > 0) {
        // fprintf(stderr, "bytesread: %d\n", bytesRead);
        if ((bytesWrite = write_buffer(STDOUT_FILENO, buf, bytesRead)) == -1) {
            fprintf(stderr, "Unable to write to stdout\n");
            exit(1);
        }
        // fprintf(stderr, "byteswrite %d\n", bytesWrite);
    }

    // if (bytesRead < 0) {
    //     fprintf(stderr, "Bytes read is less than zero\n");
    //     exit(1);
    // }

    close(fd);
}




void set(const char *location, int length, const char *content, int content_length) {


    // char buffer[MAX_BUFFER_SIZE];

    int fd = open(location, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        write(STDERR_FILENO, "Invalid Command: Unable to open set\n", sizeof("Invalid Command: Unable to open set\n") - 1);
        exit(1);
    }

    // printf("content length : %d\n", content_length);
    // printf("length : %d\n", length);

    if (content == NULL || content_length == 0)   {
        printf("OK\n");
        exit(0);
    }

    ssize_t bytesWritten = write(fd, content, MIN(content_length, length));
    // printf("Set - bytesWritten: %zd\n", bytesWritten);

    int bytesRemaining = length - bytesWritten;
    // printf("Set - bytesRemaining: %d\n", bytesRemaining);

    while ((bytesRead = read_input()) > 0 && (bytesRemaining > 0)) {
        bytesRemaining = bytesRemaining - bytesRead;
        bytesWritten = write(fd, inputLine, bytesRead);

        if (bytesWritten == -1) {
            write(STDERR_FILENO, "Invalid Command: error bytes written\n", sizeof("Invalid Command: error bytes written\n") - 1);
            close(fd);
            exit(1);
        }
        // printf("Set - bytesWritten1: %zd\n", bytesWritten);

    }
    // if (bytesWritten == -1) {
    //     write(STDERR_FILENO, "Invalid Command: error bytes written\n", sizeof("Invalid Command: error bytes written\n") - 1);
    //     close(fd);
    //     exit(1);
    // }

    if (bytesRead == 0 && bytesRemaining == 1) {
        // we should append a null line as this is striped by strtok()
        write(fd, "\n", 1);
    }
    // // Append a newline character
    // if (write(fd, "\n", 1) == -1) {
    //     perror("Error writing newline to file");
    //     close(fd);
    //     exit(1);
    // }

    // printf("Set - Content being written: %d, %s\n", length, content);

    

    close(fd);
    printf("OK\n");
    exit(0);
}




int main() {

    int bytesRead = read_input();

    // fprintf(stderr, "Main - bytesRead: %d\n", bytesRead);
    // fprintf(stderr, "Main - inputline: %s------\n", inputLine);
    // fprintf(stderr, "Main - inputline: %lu------\n", strlen(inputLine));

    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            fprintf(stderr, "Invalid Command: stdin was closed before user provided a location\n");
        } else {
            printf("error reading command\n");
        }
        exit(1);
    }


    // Ensure null-termination of the input line
    // inputLine[bytesRead] = '\0';
    strcpy(original_input, inputLine);
    char *saveptr;
    char *token = strtok_r(inputLine, "\n", &saveptr);
    char* location;
    if (token != NULL) {
        if (strcmp(token, "get") == 0) {
            // Handle get command
            location = strtok_r(NULL, "\n", &saveptr);
            if (location == NULL) {
                fprintf(stderr, "Invalid Command: no location given after get\n");
            }

            // token = strtok(NULL, "\n");
            // printf("extra token = %s\n", token);
            // if (token != NULL) {
            //     fprintf(stderr, "Invalid Command: extra command after get\n");
            //     exit(1);
            // }
            if ((bytesRead > 1)  && ( original_input[bytesRead-1] != '\n')) {
                    fprintf(stderr, "Invalid Command\n");
                    exit(1);
            }
            get(location);
        } else if (strcmp(token, "set") == 0) {
            // Handle set command
            char* location = strtok_r(NULL, "\n", &saveptr);
            // printf("Main - location: %s\n", location);
            if (location == NULL) {
                fprintf(stderr, "Invalid Command: missing location after set\n");
                exit(1);
            }
            
            // printf("location = %s\n", location);

            // Read the content length and content
            token = strtok_r(NULL, "\n", &saveptr);
            if (token == NULL) {
                fprintf(stderr, "Invalid Command: missing content length after set\n");
                exit(1);
            }
            // printf("Main - token: %s\n", token);
            int contentLength = atoi(token);


            // Check if the last character is a newline
            if (contentLength < 0) {
                fprintf(stderr, "Invalid Command: negative content length given\n");
                exit(1);
            }

            // printf("contentLength = %d\n", contentLength);
            int bytes_tokenized = saveptr - inputLine;
            int bytes_remaining = bytesRead - bytes_tokenized;

            // printf("bytes tokentized: %d\n", bytes_tokenized);
            // printf("bytes remaining in the buffer: %d\n", bytes_remaining);
        
            set(location, contentLength, saveptr, bytes_remaining);
        } else {
            fprintf(stderr, "Invalid Command\n");
            // fprintf(stderr, "Invalid command, not get or set command\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "Invalid command, no input given\n");
        exit(1);
    }

    return 0;
}


