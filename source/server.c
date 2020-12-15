#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>

char HEADER_HTTP[] = "HTTP/1.1 ";
char HEADER_LENGTH[] = "content-length: ";
char HEADER_TYPE[] = "content-type: ";

char HTTP_VALUE[2][6] = {
    "200\r\n",
    "404\r\n"
};

char TYPE_VALUE[1][100] = {
    "text/html\r\n"
};

char EMPTY_STRING[] = "\r\n";

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_SETSOCKETOPT,
    ERR_BIND,
    ERR_LISTEN
};

int init_socket(int port) {
    int server_socket, socket_option = 1;
    struct sockaddr_in server_address;

    //open socket, return socket descriptor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Fail: open socket");
        exit(ERR_SOCKET);
    }

    //set socket option
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option, (socklen_t) sizeof socket_option);
    if (server_socket < 0) {
        perror("Fail: set socket options");
        exit(ERR_SETSOCKETOPT);
    }

    //set socket address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *) &server_address, (socklen_t) sizeof server_address) < 0) {
        perror("Fail: bind socket address");
        exit(ERR_BIND);
    }

    //listen mode start
    if (listen(server_socket, 5) < 0) {
        perror("Fail: bind socket address");
        exit(ERR_LISTEN);
    }
    return server_socket;
}

char *get_word(int client_socket) {
    char *answ = NULL;
    char ch;
    int size = 0;
    for (read(client_socket, &ch, 1);
                ch != ' ' && ch != '\n' && ch != '\0';
                read(client_socket, &ch, 1)) {
        size++;
        answ = realloc(answ, (size + 1) * sizeof(char));
        answ[size - 1] = ch;
    }
    if (answ)
        answ[size] = '\0';
    else
        return get_word(client_socket);
    return answ;
}

char *get_path(int client_socket) {
    char *path;
    char *cont = get_word(client_socket);  // its important to use container in order to prevent overflow
    if (strcmp("GET", cont) ) { 
        perror("incorrect query #1");
    }
    free(cont);
    
    path = get_word(client_socket);
    
    cont = get_word(client_socket);
    if (strcmp("HTTP/1.1", cont)) {
        perror("incorrect query #2");
    }
    free(cont);

    cont = get_word(client_socket);
    if (strcmp("Host:", cont)) {
        perror("incorrect query #3");
    }
    free(cont);

    cont = get_word(client_socket);
    if (strcmp("127.0.0.1:5000", cont)) {
        perror("incorrect query #4");
    }
    free(cont);

    return path;
}

int request_is_text(char *path) {
    int i = 0;
    for ( ; path[i] != '\0' && path[i] != '.'; i++) {
    }
    if (i != strlen(path))
        return 1;
    return 0;
}

int size_long(long inp) {
    int answ = 0;
    for ( ; inp ; inp /= 10)
        answ++;
    return answ;
}

char *append(char *dest, char *src) {
    dest = realloc(dest, sizeof(char) * (strlen(dest) + strlen(src) - 1));
    strcat(dest, src);
    return dest;
}

char *get_file_size(char *path) {
    char *answ = NULL;
    struct stat stats;
    if (stat(path, &stats) != 0) {
        perror("stat error");
    }
    long size = stats.st_size;
    int length = 0;
    for (int size_copy = size; size_copy; length++) {
        size_copy /= 10;
    }
    answ = malloc(length);
    sprintf(answ, "%ld", size);
    // answ = append(answ, EMPTY_STRING);
    return answ;
}

char *get_file_content(char *path, int fd) {
    struct stat stats;
    stat(path, &stats);
    char *buff = malloc(stats.st_size);
    read(fd, buff, stats.st_size);
    return buff;
}

char *get_response_text(char *request_path) {
    int fd = open(request_path, O_RDONLY, 0);
    char *answ = NULL;
    if (fd < 0) {
        answ = malloc(strlen(HEADER_HTTP));
        strcpy(answ, HEADER_HTTP);
        answ = append(answ, HTTP_VALUE[1]);
        close(fd);
        return answ;
    }
    char *size_string = get_file_size(request_path);
    char *content = get_file_content(request_path, fd);
    answ = malloc(strlen(HEADER_HTTP));
    strcpy(answ, HEADER_HTTP);
    answ = append(answ, HTTP_VALUE[0]);
    answ = append(answ, HEADER_TYPE);
    answ = append(answ, TYPE_VALUE[0]);
    answ = append(answ, HEADER_LENGTH);
    answ = append(answ, size_string);
    answ = append(answ, EMPTY_STRING);
    answ = append(answ, EMPTY_STRING);
    answ = append(answ, content);
    free(size_string);
    free(content);
    close(fd);
    return answ;
}

int send_to_client(int client_socket, char *response) {
    if ( write(client_socket, response, strlen(response)) > 0)
        return 1;
    return 0;
}

int request_is_bin(char *request_path) {
    int i = 0;
    for ( ; request_path[i] != '.' ; i++) 
    if (i == strlen(request_path))
        return 1;
    return 0;
}

int telnet_bin(int client_socket, char *request_path) {
    int pid = fork();
    if (pid == 0) {
        dup2(client_socket, 1);
        char *cmd = malloc(sizeof(char) * 3);
        int res = execl(request_path, request_path, NULL);
        if (res == -1) {
            char head[] = "HTTP/1.1 404\r\n";
            write(1, head, strlen(head));
        }
        exit(0);
    }
}

int interaction_client(int client_socket) {
    char *request_path = get_path(client_socket);
    if (request_is_text(request_path)) {
        char *response = get_response_text(request_path + 1);
        printf("%s\n", response);
        send_to_client(client_socket, response);
        free(response);
        free(request_path);
        return 1;
    }
    if (request_is_bin(request_path)) {
        telnet_bin(client_socket, request_path);
        free(request_path);
        return 1;
    }
}

int main(int argc, char** argv) {
    struct sockaddr_in client_address;
    socklen_t size = sizeof client_address;

    if (argc != 3) {
        puts("Incorrect args.");
        puts("./server <port> <client_number>");
        puts("Example:");
        puts("./server 5000 3");
        return ERR_INCORRECT_ARGS;
    }

    int port = atoi(argv[1]);
    int client_num = atoi(argv[2]);
    int server_socket = init_socket(port);
    int client_socket;
    int *pid = malloc(sizeof(int) * client_num);

    for (int i = 0 ; i < client_num ; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            while (1) {
                puts("wait for connection");
                client_socket = accept(server_socket,
                                (struct sockaddr *) &client_address,
                                &size);
                printf("connected: %s %d\n", inet_ntoa(client_address.sin_addr),
                                ntohs(client_address.sin_port));
                interaction_client(client_socket);
                close(client_socket);
            }
            exit(0);
        }
    }

    for (int i = 0; i < client_num ; i++) {
        waitpid(pid[i], 0, 0);
    }
    return 0;
}
