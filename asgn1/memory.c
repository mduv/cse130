#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_INPUTLINE_SIZE 4096
#define MAX_BUFFER_SIZE 4096


/* HANDLE GET */
void get(const char *location) {
    // open file @ location for reading
    int fd = open(location, O_RDONLY);
    if (fd == -1) {
        perror("Unable to open file");
        exit(1);
    }

    // read and write to stdout
    char buf[MAX_BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = read(fd, buf, MAX_BUFFER_SIZE)) > 0) {
        write(1, buf, bytesRead);
    }

    write(1, buf, bytesRead);

    // close file
    close(fd);
}


int main(void) {
    /* READ INPUT COMMAND FROM STDIN */
    // char* inputLine = (char*)calloc(100, sizeof(char));
    char inputLine[MAX_INPUTLINE_SIZE];
    int bytesRead = read(0, inputLine, 4096);
    if (bytesRead <= 0) {
        perror("Something went wrong");
        exit(1);
    }


    /* Parse out the command and the file name ( " get\nfoo.txt\n "), seperate by new line */

    // get the first token (aka the command)
    char* token = strtok(inputLine, "\n");

    while (token != 0) {
        // see if it's a get command
        if (strcmp(token, "get") == 0) {
            token = strtok(NULL, "\n");
            if (token != NULL) {
                get(token);
            } else {
                perror("Invalid get command");
            }
        }
        // printf( "%s\n", token);
        // get next token
        token = strtok(NULL, "\n");
    }

    return 0;
    

}



