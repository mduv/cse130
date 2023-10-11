#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/stat.h>


#define MAX_INPUTLINE_SIZE 4096
#define MAX_BUFFER_SIZE 4096

int isDir(const char* fileName)
{
    struct stat path;

    stat(fileName, &path);

    return S_ISREG(path.st_mode);
}

/* HANDLE GET */
void get(const char *location) {
    if (strlen(location) > PATH_MAX) {
        write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
        exit(1);
    }

    // open file @ location for reading
    int fd = open(location, O_RDONLY);
    if (fd == -1) {
        write(STDERR_FILENO, "File does not exist\n", sizeof("File does not exist\n") - 1);
        exit(0);
    }

    // read and write to stdout
    char buf[MAX_BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = read(fd, buf, MAX_BUFFER_SIZE)) > 0) {
        if (bytesRead > 0 && buf[bytesRead - 1] != '\n') {
            close(fd);
            exit(1);
        }
        // buf[bytesRead] = '\0';
        write(1, buf, bytesRead);
    }

    // close file
    close(fd);
}

void set(const char *location, int length, const char* content) {
    // open file for write, create if it doen;t exist
    // printf("this far\n");
    int fd = open(location, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("Unable to open file\n");
        exit(0);
    }


    // write content to file
    // write(fd, content, length);
    int bytesWritten = write(fd, content, length);
    if (bytesWritten != length) {
        printf("bytesWritten do not match content length\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        exit(EXIT_FAILURE);
    }

    // close file
    close(fd);

    // Output success message
    write(STDOUT_FILENO, "OK\n", 3);

    // Debugging information
    // printf("Location: %s, Length: %d, Content: %.*s\n", location, length, length, content);
    
}


#define MAX_INPUTLINE_SIZE 4096

char* readLine() {
    static char line[MAX_INPUTLINE_SIZE];
    int bytesRead = read(0, line, sizeof(line));
    printf("bytes read: %d\n", bytesRead);

    if (bytesRead <= 0) {
        if (bytesRead == 0) {            
            write(STDERR_FILENO, "Invalid command: 0 bytes read\n", sizeof("Invalid command: 0 bytes read\n") - 1);
        } else {
            printf("error reading command\n");
        }
        exit(1);
    }

    return line;
}


void parseCommand(char* line) {
    // get the first token (aka the command)
    char* token = strtok(line, "\n");

    while (token != 0) {
        
        // see if it's a get command
        if (strcmp(token, "get") == 0) {
            
            char* location = strtok(NULL, "\n");

            // if location exists run handle get, else write to stderr
            if (location != NULL) {
                get(location);
            } else {
                write(STDERR_FILENO, "Invalid Command location does not exist\n", sizeof("Invalid Command location does not exist\n") - 1);
                exit(0);
            }
        } else if (strcmp(token, "set") == 0) {
            char* location = strtok(NULL, "\n");
            char* lengthStr = strtok(NULL, "\n");

            if (location == NULL || lengthStr == NULL) {
                write(STDERR_FILENO, "Invalid Command: no location or content length given\n", sizeof("Invalid Command: no location or content length given\n") - 1);
                exit(0);
            }

            size_t length = atoi(lengthStr);

            if (length <= 0) {
                write(STDERR_FILENO, "Invalid Command: invalid content length\n", sizeof("Invalid Command: invalid content length\n") - 1);
                exit(0);
            }

            char* content = strtok(NULL, ""); // Read the rest of the line as content

            if (content == NULL || strlen(content) < length) {
                write(STDERR_FILENO, "Invalid Command: no content given or insufficient content\n", sizeof("Invalid Command: no content given or insufficient content\n") - 1);
                exit(0);
            }

            set(location, length, content);
        } else {
            write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
            exit(0);
        }

        // Read the next line
        line = readLine();
        token = strtok(line, "\n");
    }
}

int main() {
    char* line = readLine();
    parseCommand(line);

    return 0;
}

// int main(void) {
//     /* READ INPUT COMMAND FROM STDIN */
//     char inputLine[MAX_INPUTLINE_SIZE];
//     int bytesRead = read(0, inputLine, 4096);

//     if (bytesRead <= 0) {
//         if (bytesRead == 0) {
//             write(STDERR_FILENO, "Invalid command: stdin was closed before user provided a location\n", sizeof("Invalid command: stdin was closed before user provided a location\n") - 1);
//         } else {
//             printf("error reading command\n");
//         }
//         exit(0);
//     }


//     /* Parse out the command and the file name ( " get\nfoo.txt\n "), seperate by new line */

//     // get the first token (aka the command)
//     char* token = strtok(inputLine, "\n");

//     while (token != 0) {
//         // see if it's a get command
//         if (strcmp(token, "get") == 0) {
//             // get location
//             token = strtok(NULL, "\n");
//             // char* token2 = strtok(NULL, "\n");
//             // if (token2 == NULL) {
//             //     write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
//             //     exit(1);
//             // }

//             // if location exists run handle get, else write to stderr
//             if (token != NULL) {
//                 get(token);
//             } else {
//                 write(STDERR_FILENO, "Invalid Command location does not exist\n", sizeof("Invalid Command location does not exist\n") - 1);
//                 exit(0);
//             }
//         } else if (strcmp(token, "set") == 0) {
//             token = strtok(NULL, "\n");

//             if (token != NULL) {
//                 // get location,
//                 char* location = token;
//                 // printf("%s\n", location);

//                 // get content_length
//                 token = strtok(NULL, "\n");
//                 // printf("%s\n", token);
//                 // printf("%d\n", token != NULL);


//                 if (token == NULL) {
//                     write(STDERR_FILENO, "Invalid Command: no content length given\n", sizeof("Invalid Command: no content length given\n") - 1);
//                     // printf("Invalid Command: no content length given\n");
//                     exit(0);
//                 }
//                 int length = atoi(token);
//                 // printf("%d\n", length);


//                 // get content
//                 token = strtok(NULL, "\n");
//                 if (token == NULL) {
//                     write(STDERR_FILENO, "Invalid Command: no content given\n", sizeof("Invalid Command: no content given\n") - 1);
//                     set(location, length, token);
//                     exit(0);
//                     // printf("%s\n", token);
//                 }
//                 char* content = token;
                

//                 // content[length] = '\0'; 
//                 set(location, length, content);
//                 // break;
//             } else {
//                 perror("Invalid Command\n");
//                 exit(0);
//             } 
//         } else {
//             // printf("%s\n", token);
//             write(STDERR_FILENO, "Invalid Command\n", sizeof("Invalid Command\n") - 1);
//             exit(0);
//         }
    
//     }
//     return 0;
// }




