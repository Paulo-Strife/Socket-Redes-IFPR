#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_LEN 2048

int main () {
    int server_fd, client_fd;

    struct sockaddr_in address;

    int addrlen = sizeof(address);

    char buffer [BUFFER_LEN];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar o socket, tente novamente");
        exit(EXIT_FAILURE);
    }

    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erro no bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Erro no listen");
        exit(EXIT_FAILURE);
    }

    printf("Server para responder ao tempo iniciado, porta%d\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

        if (client_fd < 0) {
            continue;
        }

        memset(buffer, 0, BUFFER_LEN);

        read(client_fd, buffer, sizeof(buffer));

        printf("\nMensagem recebida!%s\nserver", buffer);

        if (strcmp(buffer, "TIME") == 0) {
            struct timespec ts;

            clock_gettime(CLOCK_REALTIME, &ts);

            char response[BUFFER_LEN];

            snprintf(response, sizeof(response), "%ld %ld", ts.tv_sec, ts.tv_nsec);

            printf("Enviando horário: %s\n", response);

            send(client_fd, response, strlen(response), 0);
        } else {
            char erro[] = "comando invalido";

            send(client_fd, erro, strlen(erro), 0);
        }
        close(client_fd);
    }
    close(client_fd);

    return 0;
}