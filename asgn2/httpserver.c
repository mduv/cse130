#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <regex.h>


#include "asgn2_helper_funcs.h"
#define MAX_BUFFER_SIZE 2048

#include <regex.h>



void send_response(int socket, int status_code, char *status_text, char *content, size_t content_length) {
    // Assuming a simple HTTP/1.1 response format
    char response[100];

    sprintf(response, "HTTP/1.1 %d %s\r\nContent-Length: %zu\r\n\r\n", status_code, status_text, content_length);

    printf("response length: %lu, content length: %zu\n", strlen(response), content_length);

    

    // Send the response to the client
    write(socket, response, strlen(response));

    write_n_bytes(socket, content, content_length);

}

// size_t get_content_length(const char *headers) {
//     const char *content_length_str = strstr(headers, "Content-Length: ");
//     if (content_length_str) {
//         // Skip the "Content-Length: " prefix
//         content_length_str += strlen("Content-Length: ");
//         return atoi(content_length_str);
//     }
//     return 0;  // Default to 0 if not found or parsing fails
// }

// Function to validate the URI
int validate_uri(const char *uri) {
    
    // Check if URI length is between 2 and 64 characters
    size_t uri_length = strlen(uri);
    if (uri_length < 2 || uri_length > 64)
        return 1;

    // Check if URI contains only valid characters [a-zA-Z0-9.-]
    regex_t compiledRegex;
    int reti;
    int actualReturnValue = -1;
    // char messageBuffer[100];

    /* Compile regular expression */
    reti = regcomp(&compiledRegex, "/[a-zA-Z0-9.-]*$", REG_EXTENDED | REG_ICASE);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return -2;
    }

    /* Execute compiled regular expression */
    reti = regexec(&compiledRegex, uri, 0, NULL, 0);
    if (!reti) {
        // match
        actualReturnValue = 0;
    } else if (reti == REG_NOMATCH) {
        // puts("No match");
        actualReturnValue = 1;
    } else {
        // regerror(reti, &compiledRegex, messageBuffer, sizeof(messageBuffer));
        // fprintf(stderr, "Regex match failed: %s\n", messageBuffer);
        actualReturnValue = -3;
    }

    /* Free memory allocated to the pattern buffer by regcomp() */
    regfree(&compiledRegex);
    return actualReturnValue;
}


int useRegex(char* textToCheck) {
    regex_t compiledRegex;
    int reti;
    int actualReturnValue = -1;
    // char messageBuffer[100];

    /* Compile regular expression */
    reti = regcomp(&compiledRegex, "^HTTP/[0-9]\\.[0-9]$", REG_EXTENDED | REG_ICASE);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return -2;
    }

    /* Execute compiled regular expression */
    reti = regexec(&compiledRegex, textToCheck, 0, NULL, 0);
    if (!reti) {
        // match
        actualReturnValue = 0;
    } else if (reti == REG_NOMATCH) {
        // puts("No match");
        actualReturnValue = 1;
    } else {
        // regerror(reti, &compiledRegex, messageBuffer, sizeof(messageBuffer));
        // fprintf(stderr, "Regex match failed: %s\n", messageBuffer);
        actualReturnValue = -3;
    }

    /* Free memory allocated to the pattern buffer by regcomp() */
    regfree(&compiledRegex);
    return actualReturnValue;
}

int validate_version(char *version) {
    printf("Version: %s\n", version);
    if (useRegex(version) == 0) {
        // printf("Valid\n");
        return 1;
    } else {
        // printf("Invalid, regex returned: %d\n", useRegex(version));
        return 0;
    }
}

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

int is_end_of_token(char* request_buffer) {
    int buf_len = strlen(request_buffer);
    if (buf_len >= 2) {
        if (strcmp(&request_buffer[buf_len - 2], "\r\n") == 0) {
            return 1;
        }
    }
    return 0;
}

char* read_token(int client_socket, char* request_buffer) {
    int bytesRead;
    char *substring = malloc(MAX_BUFFER_SIZE);
    char token[MAX_BUFFER_SIZE] = {0};

    printf("read_token\n");

    while ((bytesRead = read(client_socket, request_buffer, 1)) >= 0) {
        printf("bytesRead: %d\n", bytesRead);
        strcat(token, request_buffer);
        if (is_end_of_token(token) == 1) {
            memcpy(substring, token, strlen(token) - 2);
            // Null-terminate the substring
            substring[strlen(substring)] = '\0';
            printf("substring: %s\n", substring );

            return substring;
        }
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }

    // Initialize the listener socket
    Listener_Socket listener;
    if (listener_init(&listener, port) != 0) {
        fprintf(stderr, "Failed to initialize listener\n");
        return 1;
    }

    // Main server loop
    while (1) {
        // Accept a new connection
        int client_socket = listener_accept(&listener);
        if (client_socket == -1) {
            fprintf(stderr, "Failed to accept connection\n");
            continue;
        }

        // printf("Accepted connection: %d\n", client_socket);


        // Process the connection
        char request_buffer[MAX_BUFFER_SIZE];
        // read token
        char* request_line = read_token(client_socket, request_buffer);
        // printf("request line: %s\n", request_line);

        // Parse the request line
        char method[MAX_BUFFER_SIZE];
        char uri[MAX_BUFFER_SIZE];
        char version[MAX_BUFFER_SIZE];
        int parsed = sscanf(request_line, "%s %s %s", method, uri, version);


        printf("method: %s, uri: %s, version: %s, parsed: %d\n", method, uri, version, parsed);
        request_line = read_token(client_socket, request_buffer);
        if (!request_line) {
            continue;
        }
        printf("request_line: %s len: %lu\n", request_line, strlen(request_line));

        // Read headers
        if (request_line)
        printf("request_line: %s len: %lu\n", request_line, strlen(request_line));

        // validate version
        if (validate_version(version) != 1) {
            // printf("Invalid version\n");
            send_response(client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
            close(client_socket);
            printf("closed connection\n");
            continue;
        } else if (strcmp(version, "HTTP/1.1") != 0) {
            // printf("unsupported version\n");
            send_response(client_socket, 505, "Version Not Supported", "Version Not Supported\n", strlen("Version Not Supported\n"));
            sleep(2);
            close(client_socket);
            continue;
        }

        // validate uri
        if (validate_uri(uri) != 0) {
            send_response(client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
            sleep(2);
            close(client_socket);
            continue;
        }

        if (strcmp(method, "GET") == 0) {
            // Your code for handling a valid GET request goes here

                // Open the file for reading
                int file_fd = open(uri + 1, O_RDONLY);

                if (file_fd != -1) {

                    // check if directory
                    if (is_directory(file_fd) != 0) {
                        send_response(client_socket, 403, "Forbidden", "Forbidden\n", strlen("Forbidden\n"));
                    }

                    // File exists, read its content
                    off_t file_size = lseek(file_fd, 0, SEEK_END);
                    lseek(file_fd, 0, SEEK_SET);

                    // Allocate a buffer for the file content
                    char *file_content = malloc(file_size + 1);

                    if (file_content) {
                        // Read the file content
                        ssize_t bytes_read = read_n_bytes(file_fd, file_content, file_size);
                        // printf("number of bytes read: %zd\n", bytes_read);
                        if (bytes_read != -1) {
                            // Null-terminate the content
                            file_content[bytes_read] = '\0';  
                            // Close the file
                            close(file_fd);
                            // Send the HTTP response
                            send_response(client_socket, 200, "OK", file_content, file_size);
                            // Free the allocated memory
                            free(file_content);
                        } else {
                            // Error reading file
                            close(file_fd);
                            fprintf(stderr, "Error reading file: %s\n", strerror(errno));

                            // Send an internal server error response
                            send_response(client_socket, 500, "Internal Server Error", "Internal Server Error\n", 21);
                        }
                    } else {
                        // Memory allocation failed
                        close(file_fd);
                        // Send an internal server error response
                        send_response(client_socket, 500, "Internal Server Error", "Internal Server Error\n", 21);
                    }
                } else {
                    // File does not exist
                    // Send a not found response
                    close(file_fd);
                    send_response(client_socket, 404, "Not Found", "Not Found\n", 10);
                }
        } else if (strcmp(method, "PUT") == 0) {
            printf("tyring header\n");
            char* header = read_token(client_socket, request_buffer);
            printf("header: %s\n", header);
            exit(1);
        } else {
            // Unsupported method
            // fprintf(stderr, "Unsupported method: %s\n", method);
            // Your code for sending an appropriate response goes here
            send_response(client_socket, 501, "Not Implemented", "Not Implemented\n", strlen("Not Implemented\n"));
            exit(1);
        }
        close(client_socket);
    }

    return 0;
}





// #include <stdio.h>
// #include <errno.h>
// #include <string.h>
// #include <ctype.h> 
// #include <stdbool.h>
// #include <stdlib.h>
// #include "asgn2_helper_funcs.h"

// void invalid_port(){
//     fprintf(stderr, "Invalid Port\n\n");
//     exit(1);
// }

// int read_sock_raw(char* buf, int sock_fd){
//     // printf("Here's buf: %s\n", buf);
//     // printf("I recognize sock_ptr %p\n", (void*)sock_ptr);
//     int bytes_read = read_n_bytes(sock_fd, buf, 6);

//     printf("I read %d bytes\n", bytes_read);

//     if (bytes_read == -1) {
//         fprintf(stderr, "Error reading from socket\n");

//     }

//     printf("This is what I read: %s\n", buf);
//     printf("Errno from read_sock_raw: %d\n", errno);

    

//     return 1;
// }

// int main(int argc, char* argv[]) {

//     // get port num from cmd line
//     if(argc != 2){
//         invalid_port();
//     }
//     char* portSTR = argv[1];
//     int length = strlen(portSTR);
//     for(int i=0; i<length; i++){
//         if(isdigit(portSTR[i]) == false){
//             invalid_port();
//         }
//     }
//     int portNum = atoi(portSTR);
//     if(portNum < 1 || portNum > 65535){
//         invalid_port();
//     }
//     // ^^ validate the port num

//     // printf("port is size %d\n", length);
//     // printf("Port is %s\n", portSTR);

//     // printf("Sample regex test:\n");
//     // regexParse();

//     // alloc the sock

//     Listener_Socket input_socket;
//     Listener_Socket* ptr_to_sock = &input_socket;
//     int sample = listener_init(ptr_to_sock, portNum);

//     char buf[2048];
//     memset(buf, '\0', sizeof(buf));
//     char* ptr_to_buf = buf;

//     while(true){
//         int accept_code = listener_accept(ptr_to_sock);
//         if (accept_code != -1){
//             printf("Connection successful\n");
//         }

//         // accept_code is socket by here
//         read_sock_raw(ptr_to_buf, accept_code);
//         printf("sample: %d\nErrno %d\n", sample, errno);

//         // by here connection is established
//         // do some stuff with it

//         // clear buf at loop end
//         memset(buf, '\0', sizeof(buf));
//     }

//     printf("sample: %d\nErrno %d\n", sample, errno);
//     return 0;
// }

