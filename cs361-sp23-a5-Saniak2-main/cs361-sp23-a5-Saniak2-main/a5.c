#include "a5-pthread.h"
#include "a5.h"

static const char ping_request[] = "GET /ping HTTP/1.1\r\n\r\n";
static const char echo_request[] = "GET /echo HTTP/1.1\r\n";
static const char write_request[] = "POST /write HTTP/1.1\r\n";
static const char read_request[] = "GET /read HTTP/1.1\r\n";
static const char file_request[] = "GET /%s HTTP/1.1\r\n";
static const char stats_request[] = "GET /stats HTTP/1.1\r\n";

static const char stats_response_body[] = "Requests: %d\nHeader bytes: %d\nBody bytes: %d\nErrors: %d\nError bytes: %d";

static const char ok200_response[] = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n";
static char err404_response[] = "HTTP/1.1 404 Not Found";
static char err400_response[] = "HTTP/1.1 400 Bad Request";

static const char content_len_header[] = "Content-Length: %d";

static char written[1024] = "<empty>";
static int written_size = 7;

static const char ping_header[] = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\n";
static const char ping_body[] = "pong";

static int reqs = 0;
static int head_bytes = 0;
static int body_bytes = 0;
static int errs = 0;
static int err_bytes = 0;
int threadCount = 0;

static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t rdwr_mutex = PTHREAD_MUTEX_INITIALIZER;

static int prepare_socket(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    static struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &(server.sin_addr));

    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Error on bind");
        exit(1);
    }

    if (listen(server_socket, 100) < 0) {
        perror("Error on listen");
        exit(1);
    }

    return server_socket;
}


int create_server_socket(int port, int threads) {
    int server_socket = prepare_socket(port);

    // Initialize any global variables here
    if (threadCount > threads) {
        return 0;
    } 
    threadCount++;
    return server_socket;
}


static void send_response(int sockfd, char head[1024], int head_size, char body[1024], int body_size) {
    int head_sent = send_fully(sockfd, head, head_size, 0);
    int body_sent = 0;

    while (body_sent != body_size) {
        body_sent += send_fully(sockfd, body+body_sent, body_size-body_sent, 0);
    }

    assert(head_sent == head_size);
    assert(body_sent == body_size);

    pthread_mutex_lock(&stats_mutex);
    reqs += 1;
    head_bytes += head_size;
    body_bytes += body_size;
    pthread_mutex_unlock(&stats_mutex);
}


static void handle_ping(int sockfd, char request[2048]) {
    char head[1024];
    int head_size = 0;
    char body[1024];
    int body_size = 0;

    head_size = strlen(ping_header);
    memcpy(head, ping_header, head_size);

    body_size = strlen(ping_body);
    memcpy(body, ping_body, body_size);

    send_response(sockfd, head, head_size, body, body_size);
}


static void handle_echo(int sockfd, char request[2048]) {
    char head[1024];
    int head_size = 0;
    char body[1024];
    int body_size = 0;

    char * end = strstr(request, "\r\n\r\n");
    assert(end != NULL);

    *end = '\0';

    char * start = strstr(request, "\r\n");
    assert(start != NULL);

    start += 2;

    body_size = strlen(start);
    memcpy(body, start, body_size);

    head_size = snprintf(head, sizeof(head), ok200_response, body_size);

    send_response(sockfd, head, head_size, body, body_size);
}


static void handle_read(int sockfd, char request[2048]) {
    char head[1024];
    int head_size = 0;
    char body[1024];
    int body_size = 0;

    pthread_mutex_lock(&rdwr_mutex);
    body_size = written_size;
    memcpy(body, written, body_size);
    pthread_mutex_unlock(&rdwr_mutex);

    head_size = snprintf(head, sizeof(head), ok200_response, body_size);
    send_response(sockfd, head, head_size, body, body_size);
 }


static void handle_write(int sockfd, char request[2048]) {
    char * start = strstr(request, "\r\n\r\n");
    assert(start != NULL);
    start += 4;  // Jump over new lines

    int length = 0;
    char *ptr = strstr(request, "Content-Length: ");
    if (ptr != NULL) {
        length = atoi(ptr + 16);
    }

    assert(length != 0);

    pthread_mutex_lock(&rdwr_mutex);
    // Saves posted data
    written_size = length;
    memcpy(written, start, written_size);
    pthread_mutex_unlock(&rdwr_mutex);

    handle_read(sockfd, request);
}


static void handle_stats(int sockfd, char request[2048]) {
    char head[1024];
    int head_size = 0;
    char body[1024];
    int body_size = 0;
    
    body_size = snprintf(body, sizeof(body), stats_response_body, reqs, head_bytes, body_bytes, errs, err_bytes);
    head_size = snprintf(head, sizeof(head), ok200_response, body_size);

    send_response(sockfd, head, head_size, body, body_size);
}

static void send_error(int sockfd, char * error) {
    int len = strlen(error);
    int sent = send_fully(sockfd, error, len, 0);

    assert(sent == len);
    pthread_mutex_lock(&stats_mutex);
    errs += 1;
    err_bytes += len;
    pthread_mutex_unlock(&stats_mutex);
}

static void handle_file(int sockfd, char request[2048]) {
    char head[1024];
    int head_size = 0;
    char body[1024];
    int body_size = 0;

    static char path[128];
    int found = sscanf(request, file_request, path);
    assert(found > 0);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        send_error(sockfd, err404_response);
        close(sockfd);
        return;
    }
    
    struct stat s;
    fstat(fd, &s);

    int file_size = s.st_size;

    head_size = snprintf(head, sizeof(head), ok200_response, file_size);
    int sent = send_fully(sockfd, head, head_size, 0);

    assert(sent == head_size);

    head_bytes += head_size;

    int file_read = 0;
    int file_sent = 0;
    int total_sent = 0;

    while (total_sent < file_size) {
        file_read = read(fd, body, sizeof(body));
        file_sent = 0;
        file_sent += send_fully(sockfd, body, file_read, 0);


        while (file_sent != file_read) {
            file_sent += send(sockfd, body +file_sent, file_read-file_sent, 0);
        } 

        assert(file_sent == file_read);  // We were able to send all data

        total_sent += file_sent;
        body_bytes += file_sent;
    }

    reqs += 1;
    close(fd);
}


static void handle_client_request(int sockfd) {
    char request[2048];
    int len = recv_http_request(sockfd, request, sizeof(request), 0);

    if (len == 0) {
        // No request 
        return;
    }

    assert(len > 0);

    if (!strncmp(request, ping_request, strlen(ping_request))) {
        handle_ping(sockfd, request);
        close(sockfd);
        return;
    } else if (!strncmp(request, echo_request, strlen(echo_request))) {
        handle_echo(sockfd, request);
        close(sockfd);
        return;
    } else if (!strncmp(request, write_request, strlen(write_request))) {
        handle_write(sockfd, request);
        close(sockfd);
        return;
    } else if (!strncmp(request, read_request, strlen(read_request))) {
        handle_read(sockfd, request);
        close(sockfd);
        return;
    } else if (!strncmp(request, stats_request, strlen(stats_request))) {
        handle_stats(sockfd, request);
        close(sockfd);
        return;
    } else if (!strncmp(request, "GET ", 4)) {
        handle_file(sockfd, request);
        return;
    }
    

    send_error(sockfd, err400_response);
    close(sockfd);
}


void* handle_client(void* arg) {
    int clientSocket = (long) arg;
    int clientSockNum = (int) clientSocket;
    handle_client_request(clientSockNum);
    return NULL;
}


void accept_client(int server_socket) {
    static struct sockaddr_in client;
    static socklen_t client_size;

    memset(&client, 0, sizeof(client));
    memset(&client_size, 0, sizeof(client_size));

    int client_socket = accept(server_socket, (struct sockaddr *)&client, &client_size);
    if (client_socket < 0) {
        perror("Error on accept");
        exit(1);
    }

    pthread_t thread;
    long clientNum = (long) client_socket;
    void* clientSock = (void *) clientNum;
    pthread_create(&thread, NULL, handle_client, clientSock);
    pthread_detach(thread);
}


