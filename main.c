#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_LENGTH 1000

static volatile int keepRunning = 1;
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

void echo_loop(int new_socket) {
  char bufferRead[MAX_LENGTH];
  while (keepRunning) {
    int bytesRead = read(new_socket, bufferRead, MAX_LENGTH);
    if (bytesRead > 0) {
      write(new_socket, bufferRead, MAX_LENGTH);
    } else if (bytesRead == 0) {
      printf("La conexión se ha cerrado\n");
      intHandler(1);
    } else {
      perror("Error al leer del socket");
    }
  }
}

typedef struct {
  int server_fd;
  struct sockaddr *address;
  socklen_t *addr_size;
} accept_new_connection_args_t;
int main() {
  signal(SIGINT, intHandler);

  int server_fd = create_server_socket();
  if (server_fd == -1)
    return 1;

  struct sockaddr_in address = setup_address();

  if (bind_and_listen(server_fd, address) == -1)
    return 1;

  socklen_t addr_size = sizeof(address);
  int new_socket = accept(server_fd, (struct sockaddr *)&address, &addr_size);
  if (new_socket < 0) {
    perror("Error al aceptar la conexion");
    return 1;
  }

  echo_loop(new_socket);

  close(new_socket);
  close(server_fd);

  return 0;
}
