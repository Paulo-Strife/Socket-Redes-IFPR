#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>

// definindo porta e tamanho do buffer para ser usado
#define PORT 8080
#define BUFFER_LEN 2048

int main () {
    // criando socket
    int sockfd;

    struct sockaddr_in server_addr;
    struct timespec inicio;
    struct timespec fim;
    struct timeval tv;

    char buffer[BUFFER_LEN];
    // AF_INET = IPV4 e SOCK_DGRAM = UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erro ao criar o socket!");
        exit(EXIT_FAILURE);
    }
    // Tempo para timeout
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

    // fila de requisições
    char request[] = "TIME";

    // pega o horário do relógio do sistema
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    /*Envia requisição parra o servidor, sockfd é o socket criado pelo cliente
    request é a mensagem enviada, no caso time.
    strlen é onde vemos o tamanho da mensagem enviada.
    flag que é normalmente zero
    &server_addr corresponde ao endereço do servido
    sizeof para saber o tamanho da estrutura do endereço.*/

    sendto(sockfd, request, strlen(request), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // recebe a resposta o servidor
    int bytes_lidos = recvfrom(sockfd, buffer, BUFFER_LEN - 1, 0, NULL, NULL);

    // Se não houver resposta retorna um erro
    if (bytes_lidos < 0){
        perror("Erro ao receber resposta");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // transforma o buffer lido em uma string em C válida
    buffer[bytes_lidos] = '\0';

    // mostra exatamente quando a resposta chegou
    clock_gettime(CLOCK_MONOTONIC, &fim);


    /*Aqui calcuamos o arredondamento com o algoritmo de cristian, sabemos quanto tempo a mensagem demorou
    para chegar e então fazemos o arredondamento da mensagem para o client*/
    double rtt = (double)(fim.tv_sec - inicio.tv_sec) + (double)(fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    // pega o tempo de ida e de volta e divide por 2
    double latencia = rtt / 2.0;

    // Exibe a resposta
    printf("Horário recebido: %s\n", buffer);

    // Variável para armazenar o horário
    long sec;
    long nsec;

    // separa os valores em segundos e nanosegundos
    if (sscanf(buffer, "%ld %ld", &sec, &nsec) != 2) {
        printf("Erro ao interpretar horário recebido.\n");
        close(sockfd);
        return 1;
    }

    // transforma para uma data aceita para as funções de data
    time_t tempo = (time_t)sec;

    // Converte para data lida pelo usuário ex: 12/12/2012
    struct tm *data_hora = localtime(&tempo);

    // Se a data for nula ele retorna um erro e encerra a comunicação com o servidor
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

    // printa no terminal todos os resultados obtidos pela comunicação entre servidor e cliente
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