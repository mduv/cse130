#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/stat.h>


#define MAX_INPUTLINE_SIZE 4096
#define MAX_BUFFER_SIZE 4096


/* HANDLE GET */
void get(const char *location) {
    // open file @ location for reading
    if (strlen(location) < PATH_MAX) {
        write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
        exit(1);
    }
    int fd = open(location, O_RDONLY);
    if (fd == -1) {
        perror("Unable to open file");
        exit(0);
    }

    // read and write to stdout
    char buf[MAX_BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = read(fd, buf, MAX_BUFFER_SIZE)) > 0) {
        write(1, buf, bytesRead);
    }

    // write(1, buf, bytesRead);

    // close file
    close(fd);
}

void set(const char *location, int length, const char* content) {
    // open file for write, create if it doen;t exist
    // printf("this far\n");
    int fd = open(location, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Unable to open file");
        exit(0);
    }

    

    // write content to file
    int bytesWritten = write(fd, content, length);
    if (bytesWritten != length) {
        perror("bytesWritten do not match content length");
        close(fd);
        exit(0);
    }

    // close file
    close(fd);

    // Output success message
    write(STDOUT_FILENO, "OK\n", 3);
    
}

int main(void) {
    /* READ INPUT COMMAND FROM STDIN */
    // char* inputLine = (char*)calloc(100, sizeof(char));
    char inputLine[MAX_INPUTLINE_SIZE];
    int bytesRead = read(0, inputLine, 4096);
    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            perror("Invalid command: stdin was closed before user provided a location");
        } else {
            perror("Error reading command");
        }
        exit(1);
    }


    /* Parse out the command and the file name ( " get\nfoo.txt\n "), seperate by new line */

    // get the first token (aka the command)
    char* token = strtok(inputLine, "\n");

    while (token != 0) {
        // see if it's a get command
        if (strcmp(token, "get") == 0) {
            token = strtok(NULL, "\n");
            char* token2 = strtok(NULL, "\n");
            if (token2 == NULL) {
                write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
                exit(1);
            }
            if (token != NULL) {
                get(token);
            } else {
                write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
                exit(1);
            }
        } else if (strcmp(token, "set") == 0) {
            token = strtok(NULL, "\n");

            if (token != NULL) {
                // get location,
                char* location = token;
                // printf("%s\n", location);
                // get content_length
                token = strtok(NULL, "\n");
                // printf("%s\n", token);
                // printf("%d\n", token != NULL);
                if (token == NULL) {
                    perror("Invalid Command\n");
                    exit(0);
                }
                int length = atoi(token);
                // printf("%d\n", length);
                // get content
                token = strtok(NULL, "\n");
                if (token == NULL) {
                    perror("Invalid Command\n");
                    exit(1);
                    // printf("%s\n", token);
                    
                }
                char* content = token;
                set(location, length, content);
                break;
            } else {
                perror("Invalid Command\n");
                exit(0);
            } 
        } else {
            // printf("%s\n", token);
            write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
            exit(1);
        }
    
    }
    return 0;
}




