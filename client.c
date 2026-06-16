#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define BUFFER_LEN 2048

int main () {
    int sockfd;

    struct sockaddr_in server_addr;
    struct timespec inicio;
    struct timespec fim;
    struct timeval tv;

    char buffer[BUFFER_LEN];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erro ao criar o socket!");
        exit(EXIT_FAILURE);
    }

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    setsockopt(
        sockfd,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &tv,
        sizeof(tv)
    );

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("IP Invalido");
        exit(EXIT_FAILURE);
    };

    char request[] = "TIME";

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    sendto(sockfd, request, strlen(request), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    int bytes_lidos = recvfrom(sockfd, buffer, BUFFER_LEN - 1, 0, NULL, NULL);

    if (bytes_lidos < 0){
        perror("Erro ao receber resposta");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_lidos] = '\0';

    clock_gettime(CLOCK_MONOTONIC, &fim);

    double rtt = (double)(fim.tv_sec - inicio.tv_sec) + (double)(fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    double latencia = rtt / 2.0;

    printf("Horário recebido: %s\n", buffer);

    long sec;
    long nsec;

    if (sscanf(buffer, "%ld %ld", &sec, &nsec) != 2) {
        printf("Erro ao interpretar horário recebido.\n");
        close(sockfd);
        return 1;
    }

    time_t tempo = (time_t)sec;

    struct tm *data_hora = localtime(&tempo);

    if (data_hora == NULL) {
        printf("Erro ao converter horário.\n");
        close(sockfd);
        return 1;
    }

    /* Formata a data */
    char horario_formatado[100];

    strftime(
        horario_formatado,
        sizeof(horario_formatado),
        "%d/%m/%Y %H:%M:%S",
        data_hora
    );

    struct timespec horario_corrigido;

    horario_corrigido.tv_sec = sec;

    horario_corrigido.tv_nsec = nsec + (long)(latencia * 1000000000.0);

    /* Ajuste de overflow */
    if (horario_corrigido.tv_nsec >= 1000000000L) {

        horario_corrigido.tv_sec++;

        horario_corrigido.tv_nsec -= 1000000000L;
    }

    /*
        Converte horário corrigido
    */
    time_t tempo_corrigido = (time_t)horario_corrigido.tv_sec;

    struct tm *data_hora_corrigida = localtime(&tempo_corrigido);

    char horario_corrigido_str[100];

    strftime(
        horario_corrigido_str,
        sizeof(horario_corrigido_str),
        "%d/%m/%Y %H:%M:%S",
        data_hora_corrigida
    );

    printf("RESULTADO\n");
    printf("\nRTT: %.9f segundos\n", rtt);
    printf("\nlatência %.9f segundos\n", latencia);
    printf("\nHorário Recebido pelo servidor:\n");
    printf("%s.%09ld\n", horario_formatado, nsec);
    printf("\nHorário Corrigido (cristian): \n");
    printf("%s.%09ld\n", horario_corrigido_str, horario_corrigido.tv_nsec);

    close(sockfd);
    return 0;
}