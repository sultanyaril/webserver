#include <stdio.h>
#include <stdlib.h>
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

char *request_path(int client_socket) {
    

}

int interaction_client(int client_socket) {
    char *request_path = get_path(client_socket);
    if (request_is_text(request_path)) {
        

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
    int pid = malloc(sizeof(int) * client_num);

    for (int i = 0 ; i < client_num ; i++) {
        puts("wait for connection");
        client_socket = accept(server_socket,
                        (struct sockaddr *) &client_address,
                        &size);
        printf("connected: %s %d\n", inet_ntoa(client_address.sin_addr),
                        ntohs(client_address.sin_port));    
        pid[i] = fork();
        if (pid[i] == 0) {
            interaction_client(client_socket);
            exit(0);
        }
        close(client_socket);
    }

    while(1) {
        client_socket = accept(server_socket,
                        (struct sockaddr *) &client_address,
                        &size);
        write(client_socket, "busy", 4);
        close(client_socket);
    }

    for (int i = 0; i < client_num ; i++) {
        waitpid(pid[i], 0, 0);
    }
    return 0;
}
