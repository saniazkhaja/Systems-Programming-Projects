#define  _POSIX_C_SOURCE 200809L
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/select.h>
#include<assert.h>

#include<sys/socket.h>
#include<arpa/inet.h>

#define exit(N) {fflush(stdout); fflush(stderr); _exit(N); }

static int get_port();

int main(int argc, char** argv) {
    int port = get_port();

   // printf("Using port %d\n", port);
   // printf("PID: %d\n", getpid());

    // Make server available on port
    
    

    // Process client requests
    while (1) {
        int clientID = socket(AF_INET, SOCK_STREAM, 0);
       // printf("%d\n", clientID);

        int optval = 1; 
        setsockopt(clientID, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); 

        struct sockaddr_in saddress;
        saddress.sin_family = AF_INET;
        saddress.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &(saddress.sin_addr));
        int bindNum = bind(clientID, (struct sockaddr*) &saddress, sizeof(saddress));
        // printf("%d\n", bindNum);
        // perror("error");

        int works = listen(clientID, 1);
        // printf("%d\n", works);

        struct sockaddr_in saddress1;
        socklen_t size;
        int acceptNum = accept(clientID, (struct sockaddr*) &saddress1, &size);
        // printf("%d\n", acceptNum);
        // perror("error");


        char buffer[2048]; 
        memset(buffer, 0, 2048*sizeof(buffer[0]));
        int len = recv(acceptNum, buffer, 1024, 0); 
        char bufferCopy[2048];
        memset(bufferCopy, 0, 2048*sizeof(bufferCopy[0]));
        memcpy(bufferCopy, buffer, 2048); 
        char bufferBody[4096];
        char data[2048]; 
        int content_length = 0;
        memset(data, 0, 2048*sizeof(data[0]));
        int badReq = -1;

        if (len > 0) {
            // Check if the request includes "GET /ping"
            if (strstr(buffer, "GET /ping") != NULL) {
                // Send the response "pong"
                const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\npong";
                send(acceptNum, response, strlen(response), 0);
                badReq = 0;
            } 

            // Check if the request includes "GET /echo"
            if (strstr(buffer, "GET /echo") != NULL) {
                // Construct the response headers
                int contentLength = 0;
                // Parse the headers from the request and send them back
                char * tokLen;
                tokLen = strtok(bufferCopy, "\r\n"); // split on both \r and \n

                while (tokLen != NULL) {
                    if (strstr(tokLen, ":") != NULL) {
                        contentLength += strlen(tokLen) + 2;
                    }
                    tokLen = strtok(NULL, "\r\n");
                }
                contentLength -= 2;

                char response[1024];
                snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", contentLength);
                send(acceptNum, response, strlen(response), 0);

                // Parse the headers from the request and send them back
                char * tok;
                tok = strtok(buffer, "\r\n\r\n"); // split on both \r and \n
                while (tok != NULL) {
                    if (strstr(tok, ":") != NULL) {
                        send(acceptNum, tok, strlen(tok), 0);
                        tok = strtok(NULL, "\r\n");
                        if (tok != NULL) {
                            send(acceptNum, "\r\n", 2, 0);
                        }
                    } else {
                        tok = strtok(NULL, "\r\n");
                    }
                }
                badReq = 0;
            }

            // Check if the request includes "POST /write"
            if (strstr(buffer, "POST /write") != NULL) {
                char *ptr = strstr(buffer, "Content-Length:");
                if (ptr != NULL) {
                    content_length = atoi(ptr + 16);
                }

                ptr = strstr(buffer, "\r\n\r\n");
                if (ptr != NULL) {
                    memcpy(data, ptr + 4, content_length);
                }
               
                snprintf(bufferBody, sizeof(bufferBody), "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s", content_length, data);
                send(acceptNum, bufferBody, strlen(bufferBody), 0);
                badReq = 0;
            } else if (strstr(buffer, "POST") != NULL && strstr(buffer, "GET") == NULL) {  // can't post anything other than write so error
                badReq = 1;
            }

            // Check if the request includes "GET /read"
            if (strstr(buffer, "GET /read") != NULL) {
                char *ptr = strstr(bufferBody, "Content-Length:");
                if (ptr != NULL) {
                    char response[4096];
                    send(acceptNum, bufferBody, strlen(bufferBody), 0);
                } else {
                    const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 7\r\n\r\n<empty>";
                    send(acceptNum, response, strlen(response), 0);
                }
                badReq = 0;
            }

            // Checking for file needing to be opened and read in for Test 5. Part of GET tests
            char *pathStart = strstr(buffer, "/");
            char* pathEnd = strstr(buffer, " HTTP/");
            char path[1024];
            memset(path, 0, sizeof(path));
            strncpy(path, pathStart + 1, pathEnd - pathStart - 1);
            int fd = open(path, O_RDONLY);
            // invalid
            if (strstr(buffer, "GETGET /") != NULL || strstr(buffer, "GET/") != NULL) {
                badReq = 1;
            }
            if (fd != -1 && badReq != 1 && strstr(buffer, "GET /") != NULL) {
                struct stat st;
                fstat(fd, &st);
                char response[1024];
                snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", st.st_size);
                send(acceptNum, response, strlen(response), 0);

                char bufferTemp[1024];
                memset(bufferTemp, 0, sizeof(bufferTemp));
                int bytes_read;
                while ((bytes_read = read(fd, bufferTemp, sizeof(bufferTemp))) > 0) {
                    send(acceptNum, bufferTemp, bytes_read, 0); // send the file contents in chunks
                    memset(bufferTemp, 0, sizeof(bufferTemp));
                }
            } else if (fd == -1 && badReq != 0 && badReq != 1){
                char *message = "HTTP/1.1 404 Not Found";
                send(acceptNum, message, strlen(message), 0);
            } else if (badReq == 1) {
                char *message = "HTTP/1.1 400 Bad Request";
                send(acceptNum, message, strlen(message), 0);
            }
            close(fd);
        }
    
        close(acceptNum);
        close(clientID);
    }

    return 0;
}

static int get_port() {
    int fd = open("port.txt", O_RDONLY);
    if (fd < 0) {
        perror("Could not open port");
        exit(1);
    }

    char buffer[32];
    int r = read(fd, buffer, sizeof(buffer));
    if (r < 0) {
        perror("Could not read port");
        exit(1);
    }

    return atoi(buffer);
}

