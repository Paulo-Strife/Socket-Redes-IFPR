#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_LEN 2048

int main() {

    /* Socket do servidor */
    int server_fd;

    /* Estrutura do servidor */
    struct sockaddr_in server_addr;

    /* Estrutura do cliente */
    struct sockaddr_in client_addr;

    // sabendo o tamanho do client_addr
    socklen_t client_len = sizeof(client_addr);

    // Criando buffer
    char buffer[BUFFER_LEN];

    /*
     * SOCK_DGRAM = UDP
     * SOCK_STREAM = TCP
     */
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    /*
     * Permite reutilizar a porta sem esperar o sistema
     * liberar completamente após o encerramento.
     */
    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    /*
     * Configuração do servidor
     */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /*
     * Associa o socket à porta.
     */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        exit(EXIT_FAILURE);
    }

    printf(
        "Servidor UDP de horário iniciado na porta %d\n",
        PORT
    );

    // Como UDP Não cria conexões, enviamos os datagramas para qualquer um
    while (1) {

        memset(buffer, 0, BUFFER_LEN);

        /*
         Recebe mensagem do cliente e identifica quem enviou a mensagem
         */
        int bytes_recebidos =
            recvfrom(
                server_fd,
                buffer,
                BUFFER_LEN - 1,
                0,
                (struct sockaddr *)&client_addr,
                &client_len
            );

        if (bytes_recebidos < 0) {
            perror("Erro ao receber mensagem");
            continue;
        }

        buffer[bytes_recebidos] = '\0';

        printf(
            "\nMensagem recebida: %s\n",
            buffer
        );

        /*
         * Cliente solicitou horário.
         */
        if (strcmp(buffer, "TIME") == 0) {

            struct timespec ts;

            /*
             * Obtém horário atual do sistema.
             */
            clock_gettime(
                CLOCK_REALTIME,
                &ts
            );

            char response[BUFFER_LEN];

            /*
             * Formato:
             * segundos nanossegundos
             */
            snprintf(
                response,
                sizeof(response),
                "%ld %ld",
                ts.tv_sec,
                ts.tv_nsec
            );

            printf(
                "Enviando horário: %s\n",
                response
            );

            /*
             * Responde diretamente ao cliente
             * que enviou a requisição.
             */
            sendto(
                server_fd,
                response,
                strlen(response),
                0,
                (struct sockaddr *)&client_addr,
                client_len
            );
        }
        else {

            // retorna o erro ocorrido ao cliente.

            char erro[] = "COMANDO_INVALIDO";

            sendto(
                server_fd,
                erro,
                strlen(erro),
                0,
                (struct sockaddr *)&client_addr,
                client_len
            );
        }
    }

    // Encerra o servidor.

    close(server_fd);

    return 0;
}