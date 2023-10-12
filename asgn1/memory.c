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

void get(const char *location) {
    // if (location[strlen(location)] != '\n') {
    //     write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
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
    while ((bytesRead = read(fd, buf, MAX_BUFFER_SIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, bytesRead) == -1) {
            fprintf(stderr, "Unable to write to stdout\n");
            exit(1);
        }
    }

    // if (bytesRead < 0) {
    //     fprintf(stderr, "Bytes read is less than zero\n");
    //     exit(1);
    // }

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
            fprintf(stderr, "Invalid Command: stdin was closed before user provided a location\n");
        } else {
            printf("error reading command\n");
        }
        exit(1);
    }

    if ((bytesRead > 1)  && ( inputLine[bytesRead-1] != '\n')) {
        fprintf(stderr, "Invalid Command\n");
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
    char* location;
    // printf("Main - token: %s\n", token);
    if (token != NULL) {
        if (strcmp(token, "get") == 0) {
            // Handle get command
            location = strtok(NULL, "\n");
            if (location == NULL) {
                fprintf(stderr, "Invalid Command: no location given after get\n");
            }

            // token = strtok(NULL, "\n");
            // printf("extra token = %s\n", token);
            // if (token != NULL) {
            //     fprintf(stderr, "Invalid Command: extra command after get\n");
            //     exit(1);
            // }
            get(location);
        } else if (strcmp(token, "set") == 0) {
            // Handle set command
            char* location = strtok(NULL, "\n");
            // printf("Main - location: %s\n", location);
            if (location == NULL) {
                fprintf(stderr, "Invalid Command: missing location after set\n");
                exit(1);
            }
            

            // Read the content length and content
            token = strtok(NULL, "\n");
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




// void get() {
//     char *token;
//     const char s[2] =  "\n";
//     token = strtok(NULL, s);


// }
// void set() {
// char *token;
// const char s[2] =  "\n";
// token = strtok(NULL, s);

// }

// int main() {
//     char inputLine[MAX_INPUTLINE_SIZE];
//     int bytesRead = read(0, inputLine, MAX_INPUTLINE_SIZE);
//     if (bytesRead <= 0) {
//         printf("Something went wrong\n");
//     }
//     const char s[2] =  "\n";
//     char *token;
//     /* get the first token */
//     token = strtok(inputLine, s);
    
//     /* walk through other tokens */
//     while( token != NULL ) {
//         printf( "%s\n", token );
//         if (strcmp(token, "get") == 0 ) {
//             get();
//         } else if (strcmp(token, "set") == 0) {
//             set();
//         }  else {
//             fprintf(stderr, "Invalid Command\n");
//             exit(1);
//         }
//     }
//     return(0);
// }
