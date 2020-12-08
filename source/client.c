#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_CONNECT
};

int init_socket(const char *ip, int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *host = gethostbyname(ip);
    struct sockaddr_in server_address;

    //open socket, result is socket descriptor
    if (server_socket < 0) {
        perror("Fail: open socket");
        exit(ERR_SOCKET);
    }

    //prepare server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    memcpy(&server_address.sin_addr, host -> h_addr_list[0],
           (socklen_t) sizeof server_address.sin_addr);

    //connection
    if (connect(server_socket, (struct sockaddr*) &server_address,
        (socklen_t) sizeof server_address) < 0) {
        perror("Fail: connect");
        exit(ERR_CONNECT);
    }
    return server_socket;
}

char *get_segment(void) {
    char *answ = NULL;
    int size = 0;
    for (char ch = getchar() ; ch != ' ' && ch != '/' && ch != ':' && ch != '\n'; ch = getchar()) {
        size++;
        answ = realloc(answ, (size + 1) * sizeof(char));
        answ[size - 1] = ch;
    }
    if (answ)
        answ[size] = '\0';
    else
        return get_segment();
    return answ;
}

char *get_file_path(void) {
    char *answ = NULL;
    int size = 0;
    for (char ch = getchar() ; ch!= ' ' && ch != '\n' && ch != '\0' ; ch = getchar()) {
        size++;
        answ = realloc(answ, (size + 1) * sizeof(char));
        answ[size - 1] = ch;
    }
    if (answ)
        answ[size] = '\0';
    else
        return get_file_path();
    return answ;
}

int parse(char **real_ip, int *real_port, char **real_file) {
    char *ip;
    int port;
    char *file;
    char *cont = NULL; // container
    
    cont = get_segment(); // reading "http:"
    if (!strcmp(cont, "exit")) {
        return -1;
    }

    if (strcmp(cont, "http")) {
        perror("incorrect query #1");
    }
    free(cont);

    ip = get_segment(); // getting ip
    cont = get_segment();
    port = atoi(cont); // getting port
    free(cont);
    file = get_file_path(); // getting file
 
    *real_ip = ip;
    *real_port = port;
    *real_file = file;
    return 0;
}

int main(int argc, char **argv) {
    char *ip;
    int port, server;
    char *file;

    while (parse(&ip, &port, &file) >= 0) {
        server = init_socket(ip, port);
        write(server, "GET ", 4);
        write(server, file, strlen(file));
        write(server, " HTTP/1.1\n", 10);
        write(server, "Host: ", 6);
        ip[strlen(ip)] = '\n';
        write(server, ip, strlen(ip));
        write(server, "\n", 1);
        int pid = fork();
        if (pid == 0) {
            char ch;
            while (read(server, &ch, 1) >= 0) {
                putchar(ch);
            }
            exit(0);
        } else {
            wait(NULL);
        }
        close(server);
        free(ip);
        free(file);
    }
    return 1;
}
