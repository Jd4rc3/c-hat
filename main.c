#include "linked_list.h"
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_LENGTH 1000
#define MAX_LENGTH_PREFIX 1014
#define NAME_MAX_LENGTH 10

static volatile int keepRunning = 1;
static struct Node *head = NULL;

void intHandler(__attribute__((unused)) int dummy) { keepRunning = 0; }

int create_server_socket() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        fprintf(stderr, "Error al crear el socket del servidor");
        return -1;
    }

    return server_fd;
};

struct sockaddr_in setup_address() {
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    return address;
}

int bind_and_listen(int server_fd, struct sockaddr_in address) {
    socklen_t addr_size = sizeof(address);
    if (bind(server_fd, (struct sockaddr *) &address, addr_size) < 0) {
        fprintf(stderr, "Error getting socket");
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        fprintf(stderr, "Error setting socket as listener");
        return 1;
    }

    printf("Listening at port: %d\n", ntohs(address.sin_port));

    return 0;
}

typedef struct {
    int server_fd;
    struct sockaddr *address;
    socklen_t *addr_size;
} accept_new_connection_args_t;

typedef struct {
    int sockfd;
    unsigned long max_length;
    Node *node;
} client_args;

void *handle_incoming_messages(void *args) {
    client_args *clint_args = (client_args *) args;
    _Bool first_time = 1;

    while (keepRunning) {
        char bufferRead[MAX_LENGTH];
        char bufferToAnswer[MAX_LENGTH_PREFIX];

        if (first_time) {
            char nameBuffer[10];
            char msg[] = "What is your name (max 10 characters length)";
            write(clint_args->sockfd, msg, 45);
            read(clint_args->sockfd, nameBuffer, NAME_MAX_LENGTH);
            printf("Client %s connected\n", nameBuffer);
            first_time = 0;
            clint_args->node->client->name = malloc(strlen(nameBuffer) + 1);
            strcpy(clint_args->node->client->name, nameBuffer);
        }

        int bytesRead = read(clint_args->sockfd, bufferRead, MAX_LENGTH);

        if (bytesRead > 0) {
            Node *current = head;
            while (current != NULL) {
                if (current->client->sockfd == clint_args->sockfd) {
                    current = current->next;
                    continue;
                }

                sprintf(bufferToAnswer, "[%s]: %s", clint_args->node->client->name, bufferRead);

                write(current->client->sockfd, bufferToAnswer, MAX_LENGTH_PREFIX);
                current = current->next;
            }

            memset(bufferRead, 0, sizeof(bufferRead)); // Limpia el buffer
        } else if (bytesRead == 0) {
            printf("Client disconnected\n");
            break;
        } else {
            perror("Error reading from socket");
            break;
        }
    }

    close(clint_args->sockfd);
    delete_node(&head, clint_args->node);
    free(clint_args);

    return NULL;
}

void *accept_new_connection(void *arg) {
    accept_new_connection_args_t *args = (accept_new_connection_args_t *) arg;

    while (keepRunning) {
        int new_socket = accept(args->server_fd, args->address, args->addr_size);

        if (new_socket == -1) {
            perror("Error en accept");
            exit(EXIT_FAILURE);
        }

        client_t *client = malloc(sizeof(client_t));
        client->sockfd = new_socket;

        Node *node = new_node(client);

        client_args *clint_args = malloc(sizeof(client_args));
        clint_args->sockfd = new_socket;
        clint_args->max_length = MAX_LENGTH;
        clint_args->node = node;

        if (pthread_create(&node->thread, NULL, handle_incoming_messages,
                           clint_args)) {
            perror("Error al crear el hilo\n");
        }

        insert_node(&head, node);

        print_list(&head);

        printf("New connection\n");
    }

    return NULL;
}

int main() {
    signal(SIGINT, intHandler);
    struct sockaddr_in sockaddr = setup_address();
    socklen_t addr_size = sizeof(sockaddr);
    pthread_t new_connection_handler;

    int server_fd = create_server_socket();
    if (server_fd == -1) {
        fprintf(stderr, "Error creating server socket");
        return 1;
    }

    accept_new_connection_args_t args = {.server_fd = server_fd,
            .address = (struct sockaddr *) &sockaddr,
            .addr_size = &addr_size};

    struct sockaddr_in address = setup_address();

    if (bind_and_listen(server_fd, address) == -1)
        return 1;

    if (pthread_create(&new_connection_handler, NULL, accept_new_connection,
                       &args)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    if (pthread_join(new_connection_handler, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    };

    close(server_fd);

    return 0;
}