#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_LENGTH 1000

typedef struct {
  int client_fd;
} thread_args;

static volatile int keepRunning = 1;

void intHandler(int dummy) { keepRunning = 0; }

int create_client_socket() {
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == -1) {
    perror("Error al crear el socket del cliente");
    return -1;
  }
  return client_fd;
}

struct sockaddr_in setup_server_address() {
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(8080);
  if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
    printf("\nDirección inválida / Dirección no soportada \n");
    return server_address;
  }
  return server_address;
}

int connect_to_server(int client_fd, struct sockaddr_in server_address) {
  if (connect(client_fd, (struct sockaddr *)&server_address,
              sizeof(server_address)) < 0) {
    perror("Error al conectar con el servidor");
    return -1;
  }
  return 0;
}

void *write_loop(void *arg) {
  thread_args *args = (thread_args *)arg;
  printf("Client write file descriptor %d\n", args->client_fd);

  while (keepRunning) {
    char buffer[MAX_LENGTH];
    char recv_buffer[MAX_LENGTH];
    fgets(buffer, 100 + 1, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    printf("Texto leido: %s\n", buffer);

    send(args->client_fd, buffer, strlen(buffer), 0);
  }

  return NULL;
}

void *read_loop(void *arg) {
  thread_args *args = (thread_args *)arg;
  printf("Client read file descriptor %d\n", args->client_fd);

  while (keepRunning) {
    char recv_buffer[MAX_LENGTH];
    int bytesRead = read(args->client_fd, recv_buffer, MAX_LENGTH);

    if (bytesRead > 0) {
      printf("Respuesta del servidor: %s\n", recv_buffer);
    } else if (bytesRead == 0) {
      printf("La conexión se ha cerrado\n");
    } else {
      perror("Error al leer del socket");
    }
  }
  return NULL;
}

int main() {
  signal(SIGINT, intHandler);
  pthread_t emitter_thread;
  pthread_t receiver_thread;
  thread_args *args = malloc(sizeof(thread_args));

  int client_fd = create_client_socket();
  if (client_fd == -1)
    return 1;

  struct sockaddr_in server_address = setup_server_address();

  if (connect_to_server(client_fd, server_address) == -1)
    return 1;

  args->client_fd = client_fd;

  if (pthread_create(&emitter_thread, NULL, write_loop, args)) {
    fprintf(stderr, "Error al crear el hilo\n");
    return 1;
  }

  if (pthread_create(&receiver_thread, NULL, read_loop, args)) {
    fprintf(stderr, "Error al crear el hilo\n");
    return 1;
  }

  if (pthread_join(receiver_thread, NULL)) {
    fprintf(stderr, "Error al unir el hilo\n");
    return 2;
  };

  close(client_fd);

  return 0;
}
