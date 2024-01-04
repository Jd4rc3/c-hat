#include "linked_list.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_LENGTH 1000

static volatile int keepRunning = 1;
static struct Node *head = NULL;
void intHandler(int dummy) { keepRunning = 0; }

int create_server_socket() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd == -1) {
    perror("Error al crear el socket del servidor");
    return 1;
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
  // Bind server address to socket
  if (bind(server_fd, (struct sockaddr *)&address, addr_size) < 0) {
    perror("Error al obtener la direccion y el puerto del socket");
    return 1;
  }

  // Listen incoming connections
  if (listen(server_fd, 3) < 0) {
    perror("Error al escuchar conexiones");
    return 1;
  }

  printf("Escuchando en el puerto: %d\n", ntohs(address.sin_port));

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
  // TODO funcion para manejar los mensajes entrantes (la funcion que se encarga
  // de enviar los mensajes a todos los clientes excepto al emisor)
  client_args *clint_args = (client_args *)args;

  while (1) {
    char bufferRead[MAX_LENGTH];
    int bytesRead = read(clint_args->sockfd, bufferRead, MAX_LENGTH);
    printf("Waiting for message in socket %d\n", clint_args->sockfd);

    if (bytesRead > 0) {

      Node *current = head;
      while (current != NULL) {
        if (current->client->sockfd == clint_args->sockfd) {
          current = current->next;
          continue;
        }

        write(current->client->sockfd, bufferRead, MAX_LENGTH);
        printf("writing message in socket %d\n", current->client->sockfd);
        current = current->next;
      }

    } else if (bytesRead == 0) {
      printf("La conexión se ha cerrado\n");
      close(clint_args->sockfd);
      delete_node(&head, clint_args->node);
      break;
    } else {
      perror("Error al leer del socket");
      break;
    }
  }

  // echo_loop(clint_args->sockfd);
  return NULL;
}

void send_messages(char *message) {
  Node *next = head->next;

  while (next->next == NULL) {
    write(next->client->sockfd, message, MAX_LENGTH);
  }
}

void *accept_new_connection(void *arg) {
  accept_new_connection_args_t *args = (accept_new_connection_args_t *)arg;

  while (keepRunning) {
    int new_socket = accept(args->server_fd, args->address, args->addr_size);

    if (new_socket == -1) {
      perror("Error en accept");
      exit(EXIT_FAILURE);
    }

    client_t *client = malloc(sizeof(client_t));
    client->sockfd = new_socket;
    client->name = "Mario";

    Node *node = new_node(client);
    client_args clint_args = {
        .sockfd = new_socket, .max_length = MAX_LENGTH, .node = node};

    printf("Spawning thread for socket %d\n", new_socket);
    if (pthread_create(&node->thread, NULL, handle_incoming_messages,
                       &clint_args)) {
      fprintf(stderr, "Error al crear el hilo\n");
    }

    insert_node(&head, node);

    print_list(&head);

    printf("Nueva conexión\n");
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
    perror("Error al crear el socket del servidor");
    return 1;
  }

  accept_new_connection_args_t args = {.server_fd = server_fd,
                                       .address = (struct sockaddr *)&sockaddr,
                                       .addr_size = &addr_size};

  struct sockaddr_in address = setup_address();

  if (bind_and_listen(server_fd, address) == -1)
    return 1;

  if (pthread_create(&new_connection_handler, NULL, accept_new_connection,
                     &args)) {
    fprintf(stderr, "Error al crear el hilo\n");
    return 1;
  }

  if (pthread_join(new_connection_handler, NULL)) {
    fprintf(stderr, "Error al unir el hilo\n");
    return 2;
  };

  close(server_fd);

  return 0;
}
