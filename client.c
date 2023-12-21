#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int client_fd;
    struct sockaddr_in server_address;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Error al crear el socket del cliente");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr)<=0) {
        printf("\nDirección inválida / Dirección no soportada \n");
        return -1;
    }

    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error al conectar con el servidor");
        return 1;
    }

    char buffer[200] = "Hola, servidor!";
    send(client_fd, buffer, strlen(buffer), 0);

    char recv_buffer[200];
    int bytesRead = read(client_fd, recv_buffer, 200);
    if (bytesRead > 0) {
        recv_buffer[bytesRead] = '\0';
        printf("Respuesta del servidor: %s\n", recv_buffer);
    } else if (bytesRead == 0) {
        printf("La conexión se ha cerrado\n");
    } else {
        perror("Error al leer del socket");
    }

    close(client_fd);

    return 0;
}
