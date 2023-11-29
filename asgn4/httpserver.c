#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>


#include "rwlock.h"
#include "queue.h"

#include "asgn2_helper_funcs.h"
#define MAX_BUFFER_SIZE 2048
#define MAX_HEADERS     5

#include <regex.h>

#define MAX_MATCHES    10 // Maximum number of matches
#define MAX_HEADER_LEN 1024 // Maximum length of the header string

typedef struct {
    char *key;
    char *value;
} HeaderField;

HeaderField parse_http_header(const char *header_text) {
    regex_t regex;
    regmatch_t matches[MAX_MATCHES];
    int status;
    HeaderField header = { NULL, NULL };

    // Define the regular expression pattern
    const char *pattern = "([A-Za-z-]+): (.+)";

    // Compile the regular expression
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        perror("Regex compilation failed");
        return header;
    }

    // Execute the regular expression
    status = regexec(&regex, header_text, MAX_MATCHES, matches, 0);
    if (status == 0) {
        // Extract key and value using match positions
        int key_len = matches[1].rm_eo - matches[1].rm_so;
        int value_len = matches[2].rm_eo - matches[2].rm_so;

        // Allocate memory for key and value
        header.key = (char *) malloc(key_len + 1);
        header.value = (char *) malloc(value_len + 1);

        // Copy key and value into the allocated memory
        snprintf(header.key, key_len + 1, "%s", header_text + matches[1].rm_so);
        snprintf(header.value, value_len + 1, "%s", header_text + matches[2].rm_so);
    }

    // Free the regex resources
    regfree(&regex);

    return header;
}

// bool firsttime = true;
void send_response(
    int socket, int status_code, char *status_text, char *content, size_t content_length) {
    // Assuming a simple HTTP/1.1 response format
    char response[100];

    // if (firsttime) {
    //     firsttime = false;
    //     sleep(3);
    // }

    sprintf(response, "HTTP/1.1 %d %s\r\nContent-Length: %zu\r\n\r\n", status_code, status_text,
        content_length);

    // printf("response length: %lu, content length: %zu\n", strlen(response), content_length);

    // Send the response to the client

    // protect log and write with a lock
    // log here
    write(socket, response, strlen(response));
    write_n_bytes(socket, content, content_length);
    // end of protection
}

pthread_mutex_t orderingLock = PTHREAD_MUTEX_INITIALIZER;

void send_response_and_log(int socket, int status_code, char *status_text, char *content, size_t content_length, char *method, char *uri, int request_id) {
    // protect with lock

    pthread_mutex_lock(&orderingLock);
    // Log entry format: <Oper>,<URI>,<Status-Code>,<RequestID header value>\n
    fprintf(stderr, "%s,%s,%d,%d\n", method, uri, status_code, request_id);
    

    send_response(socket, status_code, status_text, content, content_length);

    // printf("Done send response for proccess %d\n", request_id);

    pthread_mutex_unlock(&orderingLock);
}

// Function to validate the method
int validate_method(const char *method) {
    // Check if method length is at most than 8 characters
    size_t method_length = strlen(method);
    if (method_length > 8)
        return 1;

    // Check if method contains only valid characters [a-zA-Z]
    regex_t compiledRegex;
    int reti;
    int actualReturnValue = -1;

    /* Compile regular expression */
    reti = regcomp(&compiledRegex, "[a-zA-Z]*$", REG_EXTENDED | REG_ICASE);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return -2;
    }

    /* Execute compiled regular expression */
    reti = regexec(&compiledRegex, method, 0, NULL, 0);
    if (!reti) {
        // match
        actualReturnValue = 0;
    } else if (reti == REG_NOMATCH) {
        // No match
        actualReturnValue = 1;
    } else {
        actualReturnValue = -3;
    }

    /* Free memory allocated to the pattern buffer by regcomp() */
    regfree(&compiledRegex);
    return actualReturnValue;
}

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

    /* Compile regular expression */
    reti = regcomp(&compiledRegex, "/[a-zA-Z0-9.-_]*$", REG_EXTENDED | REG_ICASE);
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
        // No match
        actualReturnValue = 1;
    } else {
        actualReturnValue = -3;
    }

    /* Free memory allocated to the pattern buffer by regcomp() */
    regfree(&compiledRegex);
    return actualReturnValue;
}

int validate_version(char *textToCheck) {
    regex_t compiledRegex;
    int reti;
    int actualReturnValue = -1;

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
        // No match
        actualReturnValue = 1;
    } else {
        actualReturnValue = -3;
    }

    /* Free memory allocated to the pattern buffer by regcomp() */
    regfree(&compiledRegex);
    return actualReturnValue;
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

int is_end_of_token(char *token) {
    int buf_len = strlen(token);
    if (buf_len >= 2) {
        if (strcmp(&token[buf_len - 2], "\r\n") == 0) {
            return 1;
        }
    }
    return 0;
}

char *read_body(int client_socket, int content_len) {
    char *body = calloc(content_len + 1, sizeof(char));

    int bytes_read = read_n_bytes(client_socket, body, content_len);
    if (bytes_read != -1) {
        body[bytes_read] = '\0';
        return body;
    } else {
        free(body);
        return NULL;
    }
}

int write_body(int file_fd, char *body, int content_len) {
    return write_n_bytes(file_fd, body, content_len);
}

char *read_token(int client_socket) {
    int bytesRead;
    char *token = calloc(MAX_BUFFER_SIZE, sizeof(char));
    char request_buffer = '\0';

    while ((bytesRead = read(client_socket, &request_buffer, 1)) >= 0) {
        strncat(token, &request_buffer, 1);

        if (is_end_of_token(token) == 1) {
            // memcpy(substring, token, strlen(token) - 2);
            // Null-terminate the substring
            token[strlen(token) - 2] = '\0';
            // printf("token: %s len:%lu\n", token, strlen(token));
            return token;
        }
    }

    // Incomplete token. Reached end of input
    printf("Incomplete token\n");
    return NULL;
}

int read_headers(int client_socket, char *headers[]) {
    int header_count = 0;
    bool end_of_headers = false;
    while (!end_of_headers) {
        char *header = read_token(client_socket);
        if (!header) {
            // Malformed headers
            return -1;
        }
        // printf("header: %s, header_count:%d\n", header, header_count);
        if ((strlen(header) > 0) && (header_count < MAX_HEADERS)) {
            headers[header_count++] = header;
        } else {
            end_of_headers = true;
        }
    }
    return header_count;
}

typedef struct Node {
    char *key;
    rwlock_t *rw_lock;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
} LinkedMap;

void linkedmap_init(LinkedMap *map) {
    map->head = NULL;
}

void linkedmap_insert(LinkedMap *map, const char *key, PRIORITY priority, int n) {
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    newNode->key = strdup(key);
    newNode->rw_lock = rwlock_new(priority, n);
    newNode->next = map->head;

    map->head = newNode;
}

rwlock_t *linkedmap_get_lock(LinkedMap *map, const char *key) {
    Node *current = map->head;
    while (current != NULL) {
        // check for current->key and key being not null (TODO)
        if (strcmp(current->key, key) == 0) {
            return current->rw_lock;
        }
        current = current->next;
    }
    return NULL;
}

void linkedmap_cleanup(LinkedMap *map) {
    Node *current = map->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;

        // check for temp->key  bein not null (TODO)

        free(temp->key);
        rwlock_delete(&temp->rw_lock);
        free(temp);
    }
}

pthread_mutex_t mapLock;

void proccess_connection(int client_socket, LinkedMap *fileLocks) {
            // Read request line
            char *request_line = read_token(client_socket);
            if (!request_line) {
                // Invalid request
                printf("Invalid request line\n");
                send_response(
                    client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
                close(client_socket);
                return;
            }
            // printf("request line: %s\n", request_line);

            // Parse the request line
            char method[MAX_BUFFER_SIZE] = { 0 };
            char uri[MAX_BUFFER_SIZE] = { 0 };
            char version[MAX_BUFFER_SIZE] = { 0 };
            int parsed = sscanf(request_line, "%s %s %s", method, uri, version);
            if (parsed < 2) {
                printf("Invalid Request Line\n");
                // Invalid request
                send_response(
                    client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
                
                close(client_socket);
                return;
            }
            // printf("method: %s, uri: %s, version: %s, parsed: %d\n", method, uri, version, parsed);

            // Read headers
            char *headers[MAX_HEADERS];
            int header_count = read_headers(client_socket, headers);
            if (header_count < 0) {
                printf("Missing headers\n");
                // Invalid request
                send_response(
                    client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
                close(client_socket);
                return;
            }
            for (int i = 0; i < header_count; i++) {
                // printf("headers[%d]: %s\n", i, headers[i]);
            }

            // validate version
            if (validate_version(version) != 0) {
                printf("Invalid version\n");
                send_response(
                    client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
                close(client_socket);
                return;
            } else if (strcmp(version, "HTTP/1.1") != 0) {
                send_response(client_socket, 505, "Version Not Supported", "Version Not Supported\n",
                    strlen("Version Not Supported\n"));
                close(client_socket);
                return;
            }

            // validate uri
            if (validate_uri(uri) != 0) {
                printf("Invalid URI\n");
                send_response(
                    client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
                close(client_socket);
                return;
            }

            // validate method
            if (validate_method(method) != 0) {
                printf("Invalid method\n");
                send_response(
                    client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
                close(client_socket);
                return;
            } else if ((strcmp(method, "PUT") != 0) && (strcmp(method, "GET") != 0)) {
                send_response(client_socket, 501, "Not Implemented", "Not Implemented\n",
                    strlen("Not Implemented\n"));
                close(client_socket);
                return;
            }

            

            if (strcmp(method, "GET") == 0) {
                // lock map
                pthread_mutex_lock(&mapLock);

                

                // get lock from map
                rwlock_t *lock = linkedmap_get_lock(fileLocks, uri);
                // printf("uri: %s\n", uri);
                if (lock == NULL) {
                    // printf("Lock is null, inserting...\n");
                    // Insert a lock into hashtable
                    linkedmap_insert(fileLocks, uri, N_WAY, 1);
                }
                // unlock map
                lock = linkedmap_get_lock(fileLocks, uri);
                pthread_mutex_unlock(&mapLock);

                
                // printf("hello\n");

                reader_lock(lock);
                // printf("I got a reader lock...\n");

                // printf("Proccessing GET connection %d....\n", client_socket);
                // sleep(3);

                // get request_id
                int request_id = 0;
                for (int i = 0; i < header_count; i++) {
                    HeaderField header = parse_http_header(headers[i]);
                    // printf("header: key: %s, value: %s\n", header.key, header.value);

                    if (strcmp(header.key, "Request-Id") == 0) {
                        request_id = atoi(header.value);
                        break;
                    }
                }
                // printf("Request-Id: %d\n", request_id);
                
                // Your code for handling a valid GET request goes here

                // Open the file for reading
                int file_fd = open(uri + 1, O_RDONLY);

                if (file_fd != -1) {

                    // check if directory
                    if (is_directory(file_fd) != 0) {
                        printf("Is a directory\n");
                        // send_response(client_socket, 403, "Forbidden", "Forbidden\n", strlen("Forbidden\n"));
                        send_response_and_log(client_socket, 403, "Forbidden", "Forbidden\n", strlen("Forbidden\n"), method, uri, request_id);
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
                            // send_response(client_socket, 200, "OK", file_content, file_size);
                            send_response_and_log(client_socket, 200, "OK", file_content, file_size, method, uri, request_id);
                            // Free the allocated memory
                            free(file_content);
                        } else {
                            // Error reading file
                            close(file_fd);
                            fprintf(stderr, "Error reading file: %s\n", strerror(errno));

                            // Send an internal server error response
                            // send_response(client_socket, 500, "Internal Server Error", "Internal Server Error\n", 21);
                            send_response_and_log(client_socket, 500, "Internal Server Error", "Internal Server Error\n", 21, method, uri, request_id);
                        }
                    } else {
                        // Memory allocation failed
                        close(file_fd);
                        // Send an internal server error response
                        // send_response(client_socket, 500, "Internal Server Error", "Internal Server Error\n", 21);
                        send_response_and_log(client_socket, 500, "Internal Server Error", "Internal Server Error\n", 21, method, uri, request_id);
                    }
                } else {
                    // File does not exist
                    // Send a not found response
                    close(file_fd);
                    // send_response(client_socket, 404, "Not Found", "Not Found\n", 10);
                    send_response_and_log(client_socket, 404, "Not Found", "Not Found\n", 10, method, uri, request_id);
                }

                // printf("Done proccessing GET %d\n", request_id);
                reader_unlock(lock);

            } else if (strcmp(method, "PUT") == 0) {

                // lock map
                pthread_mutex_lock(&mapLock);

                // get lock from map
                rwlock_t *lock = linkedmap_get_lock(fileLocks, uri);
                if (lock == NULL) {
                    // Insert a lock into hashtable
                    linkedmap_insert(fileLocks, uri, N_WAY, 1);
                }
                // unlock map
                lock = linkedmap_get_lock(fileLocks, uri);
                pthread_mutex_unlock(&mapLock);


                writer_lock(lock);

                // printf("I got a writer lock...\n");

                // printf("Proccessing PUT connection %d....\n", client_socket);
                // sleep(3);

                
                int content_len = 0;
                for (int i = 0; i < header_count; i++) {
                    HeaderField header = parse_http_header(headers[i]);
                    // printf("header: key: %s, value: %s\n", header.key, header.value);

                    if (strcmp(header.key, "Content-Length") == 0) {
                        content_len = atoi(header.value);
                        break;
                    }
                }
                // printf("Content length: %d\n", content_len);

                // get request_id
                int request_id = 0;
                for (int i = 0; i < header_count; i++) {
                    HeaderField header = parse_http_header(headers[i]);
                    // printf("header: key: %s, value: %s\n", header.key, header.value);

                    if (strcmp(header.key, "Request-Id") == 0) {
                        request_id = atoi(header.value);
                        break;
                    }
                }
                // printf("Request-Id: %d\n", request_id);

                if (content_len <= 0) {
                    printf("Content Len <= 0\n");
                    // send_response(client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"));
                    send_response_and_log(client_socket, 400, "Bad Request", "Bad Request\n", strlen("Bad Request\n"), method, uri, request_id);
                    close(client_socket);
                }

                // Your code for handling a valid PUT request goes here
                // Open the file for writing or create a new file
                // int flags = O_WRONLY | O_CREAT | O_TRUNC;

                int flags = O_WRONLY | O_TRUNC;
                int new_file = 0;

                int file_fd = open(uri + 1, flags, 0644);

                if (file_fd == -1 && errno == ENOENT) {
                    // file does not exists, create one
                    new_file = 1;
                    printf("file %s was created\n", uri + 1);
                    close(file_fd);
                    flags |= O_CREAT;
                    file_fd = open(uri + 1, flags, 0644);
                } else {
                    // printf("--------file %s is not new %d-----\n", uri + 1, errno);
                }

                if (file_fd != -1) {
                    // check if directory
                    if (is_directory(file_fd) != 0) {
                        printf("Is a directory\n");
                        // send_response(client_socket, 403, "Forbidden", "Forbidden\n", strlen("Forbidden\n"));
                        send_response_and_log(client_socket, 403, "Forbidden", "Forbidden\n", strlen("Forbidden\n"), method, uri, request_id);
                    } else {
                        char *request_body = read_body(client_socket, content_len);
                        if (request_body != NULL) {
                            write_body(file_fd, request_body, content_len);
                            // Free the allocated memory
                            free(request_body);
                            // Close the file
                            close(file_fd);
                            // Send the HTTP response

                            if (new_file == 1) {
                                // send_response(client_socket, 201, "Created", "Created\n", strlen("Created\n"));
                                send_response_and_log(client_socket, 201, "Created", "Created\n", strlen("Created\n"), method, uri, request_id);
                            } else {
                                // send_response(client_socket, 200, "Ok", "Ok\n", strlen("Ok\n"));
                                send_response_and_log(client_socket, 200, "Ok", "Ok\n", strlen("Ok\n"), method, uri, request_id);
                            }
                        }
                    }
                }

                // printf("Done proccessing PUT %d\n", request_id);
                writer_unlock(lock);
                
            }

}

void dummy_proccess(int client_socket) {
    printf("Proccessing connection %d....\n", client_socket);
    sleep(1);
    printf("Done proccessing %d\n", client_socket);
}

queue_t *requestQueue = NULL;
LinkedMap fileLocks;

// Worker function
void *worker_function() {
    while (1) {
        // Dequeue a client socket from the thread-safe queue
        int* client_socket_ptr;
        if (queue_pop(requestQueue, (void **)&client_socket_ptr)) {
            // Process the request using the client_socket
            // (Your existing code for request processing goes here)
            // Process the connection
            int client_socket = *client_socket_ptr;
            proccess_connection(client_socket, &fileLocks);
            // dummy_proccess(client_socket);
            close(client_socket);
            free(client_socket_ptr);
        }
    }
}

int main(int argc, char *argv[]) {
    int port;
    int threads = 4; // default value

    int opt;
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
            case 't':
                threads = atoi(optarg);
                printf("threads: %d\n", threads);
                break;
            default:
                fprintf(stderr, "Usage: %s [-t threads] <port>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[optind]);
    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }

    // Create and initialize a linked map
    linkedmap_init(&fileLocks);


    // Initialize the listener socket
    Listener_Socket listener;
    if (listener_init(&listener, port) != 0) {
        fprintf(stderr, "Failed to initialize listener\n");
        return 1;
    }

    // Initialize reqeuest queue
    requestQueue = queue_new(threads);




    pthread_t worker_threads[threads];

    // Worker threads
    for (int i = 0; i < threads; ++i) {
        pthread_create(&worker_threads[i], NULL, worker_function, NULL);
    }

    // Dispatcher thread
    while (1) {
        // Dynamically allocate memory for the client_socket
        int *client_socket_ptr = (int *)malloc(sizeof(int));

        if (client_socket_ptr == NULL) {
            // Malloc failure
            fprintf(stderr, "Malloc failed");
            break;
        }


        // Accept a new connection
        *client_socket_ptr = listener_accept(&listener);
        int client_socket = *client_socket_ptr;

        if (client_socket == -1) {
            fprintf(stderr, "Failed to accept connection\n");
            free(client_socket_ptr);
            continue;
        }

        // printf("Accepted connection: %d\n", client_socket);


        // Enqueue the client socket into the thread-safe queue
        if (!queue_push(requestQueue, (void *)client_socket_ptr)) {
            // Failed to enqueue, handle the error (e.g., close the socket)
            fprintf(stderr, "Failed to enqueue client socket\n");
            close(client_socket);
            free(client_socket_ptr);
        }

    }

    // Join worker threads (optional, depends on your implementation)
    for (int i = 0; i < threads; ++i) {
        pthread_join(worker_threads[i], NULL);
    }

    // Cleanup the map
    linkedmap_cleanup(&fileLocks);

    return 0;
}
