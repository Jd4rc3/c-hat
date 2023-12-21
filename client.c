#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_LENGTH 1000

static volatile int keepRunning = 1;
void intHandler(int dummy) { keepRunning = 0; }

int main() {
  signal(SIGINT, intHandler);

  int client_fd;
  struct sockaddr_in server_address;
  char buffer[MAX_LENGTH];
  char recv_buffer[MAX_LENGTH];

  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == -1) {
    perror("Error al crear el socket del cliente");
    return 1;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(8080);
  if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
    printf("\nDirección inválida / Dirección no soportada \n");
    return -1;
  }

  if (connect(client_fd, (struct sockaddr *)&server_address,
              sizeof(server_address)) < 0) {
    perror("Error al conectar con el servidor");
    return 1;
  }

  while (keepRunning) {
    fgets(buffer, 100 + 1, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    printf("Texto leido: %s\n", buffer);

    send(client_fd, buffer, strlen(buffer), 0);

    int bytesRead = read(client_fd, recv_buffer, MAX_LENGTH);
    if (bytesRead > 0) {
      printf("Respuesta del servidor: %s\n", recv_buffer);
    } else if (bytesRead == 0) {
      printf("La conexión se ha cerrado\n");
    } else {
      perror("Error al leer del socket");
    }
  }

  close(client_fd);

  return 0;
}
