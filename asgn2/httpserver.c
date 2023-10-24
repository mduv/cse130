#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>



#include "asgn2_helper_funcs.h"
#define MAX_BUFFER_SIZE 2048


void send_response(int socket, int status_code, const char *status_text, const char *content, size_t content_length) {
    // Assuming a simple HTTP/1.1 response format
    char response[2048];
    sprintf(response, "HTTP/1.1 %d %s\r\nContent-Length: %zu\r\n\r\n%s", status_code, status_text, content_length, content);

    // Send the response to the client
    write(socket, response, strlen(response));
}


// Function to validate the URI
int validate_uri(const char *uri) {
    // Check if URI starts with '/'
    if (uri[0] != '/')
        return 0;

    // Check if URI length is between 2 and 64 characters
    size_t uri_length = strlen(uri);
    if (uri_length < 2 || uri_length > 64)
        return 0;

    // Check if URI contains only valid characters [a-zA-Z0-9.-]
    for (size_t i = 1; i < uri_length; ++i) {
        char ch = uri[i];
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '.' || ch == '-'))
            return 0;
    }

    // If all checks pass, the URI is valid
    return 1;
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

        // Process the connection
        char request_buffer[MAX_BUFFER_SIZE];
        int bytes_read = read_n_bytes(client_socket, request_buffer, MAX_BUFFER_SIZE);

        printf("I read %d bytes\n", bytes_read);
        // if (bytes_read == -1) {
        //     fprintf(stderr, "Error reading from socket\n");
        // }
        printf("This is what I read: %s\n", request_buffer);
        printf("Errno from read_sock_raw: %d\n", errno);

        // if (bytes_read <= 0) {
        //     fprintf(stderr, "Error reading from client socket\n");
        //     continue;
        // }

        // Null-terminate the buffer to treat it as a string
        request_buffer[bytes_read] = '\0';

        // Parse the request and handle it
        // Parse the request line
        char *method, *uri, *version;
        int parsed = sscanf(request_buffer, "%m[^ ] %m[^ ] %m[^\r\n]", &method, &uri, &version);

        if (parsed != 3) {
            // Invalid request format
            fprintf(stderr, "Invalid request format\n");
            // Your code for sending an appropriate response goes here
            continue;
        }

        // Process the parsed information
        printf("Method: %s, URI: %s, Version: %s\n", method, uri, version);

        // Your code for handling the HTTP request goes here
        if (strcmp(method, "GET") == 0) {
            // Handle GET request
            printf("Handling GET request\n");

            // Check if the URI is valid
            if (validate_uri(uri)) {
                // Your code for handling a valid GET request goes here

                // Open the file for reading
                int file_fd = open(uri + 1, O_RDONLY);
                if (file_fd != -1) {
                    // File exists, read its content
                    off_t file_size = lseek(file_fd, 0, SEEK_END);
                    lseek(file_fd, 0, SEEK_SET);

                    // Allocate a buffer for the file content
                    char *file_content = malloc(file_size + 1);
                    if (file_content) {
                        // Read the file content
                        ssize_t bytes_read = read(file_fd, file_content, file_size);
                        if (bytes_read != -1) {
                            file_content[bytes_read] = '\0';  // Null-terminate the content

                            // Close the file
                            close(file_fd);

                            // Send the HTTP response
                            send_response(client_socket, 200, "OK", file_content, strlen(file_content));

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
                        fprintf(stderr, "Memory allocation failed\n");

                        // Send an internal server error response
                        send_response(client_socket, 500, "Internal Server Error", "Internal Server Error\n", 21);
                    }
                } else {
                    // File does not exist
                    fprintf(stderr, "File not found: %s\n", uri + 1);  // Skip the leading '/'

                    // Send a not found response
                    send_response(client_socket, 404, "Not Found", "Not Found\n", 10);
                }
                printf("Valid GET request\n");
            } else {
                // Invalid URI
                fprintf(stderr, "Invalid URI: %s\n", uri);
                // Your code for sending an appropriate response goes here
            }

            

            // Your code for handling a GET request goes here
        } else if (strcmp(method, "PUT") == 0) {
            // Handle PUT request
            printf("Handling PUT request\n");

            // Your code for handling a valid PUT request goes here

            // Open or create the file for writing
            int file_descriptor = open(uri + 1, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  // Skip the leading '/'
            if (file_descriptor != -1) {
                // Write the content to the file
                ssize_t bytes_written = write(file_descriptor, request_buffer, 57);

                if (bytes_written != -1) {
                    // Close the file
                    close(file_descriptor);

                    // Send the HTTP response
                    send_response(client_socket, 200, "OK", "OK\n", 3);

                    // Continue to the next iteration of the loop
                    continue;
                } else {
                    // Error writing to the file
                    perror("Error writing to file");
                }

                // Close the file in case of an error
                close(file_descriptor);
            } else {
                // Error opening or creating the file
                perror("Error opening/creating file");

                // Send an internal server error response
                send_response(client_socket, 500, "Internal Server Error", "Internal Server Error\n", 21);
            }

        } else {
            // Unsupported method
            fprintf(stderr, "Unsupported method: %s\n", method);
            // Your code for sending an appropriate response goes here
        }


        // Free allocated memory
        free(method);
        free(uri);
        free(version);


        // Your code for processing the connection goes here

        // Close the connection
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

