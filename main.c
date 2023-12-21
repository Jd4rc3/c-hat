#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addr_size = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1){
        perror("Error al crear el socket del servidor");
        return 1;
    }

    // Setup socket address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Bind server address to socket
    if (bind(server_fd, (struct sockaddr *)&address, addr_size) < 0){
        perror("Error al obtener la direccion y el puerto del socket");

        return 1;
    }

    printf("Direccion: %s\n", inet_ntoa(address.sin_addr));
    printf("Puerto:  %d\n", ntohs(address.sin_port));

    // Listen incoming connections
    if (listen(server_fd, 3) < 0){
        perror("Error al escuchar conexiones");
        return 1;
    }

    printf("Escuchando en el puerto: %d\n", ntohs(address.sin_port));

    // accept incoming connections
    new_socket = accept(server_fd, (struct sockaddr *)&address, &addr_size);
    if (new_socket < 0) {
        perror("Error al aceptar la conexion");
        return 1;
    }

    char bufferRead[200];
    int bytesRead = read(new_socket, bufferRead, 200);
    if (bytesRead > 0) {
        bufferRead[bytesRead] = '\0'; // Ensure buffer ends up with null (string end)
        printf("Bytes leídos: %s\n", bufferRead);

        write(new_socket, bufferRead, 200);
    } else if (bytesRead == 0) {
        printf("La conexión se ha cerrado\n");
    } else {
        perror("Error al leer del socket");
    }

    close(new_socket);
    close(server_fd);

    return 0;
}
