//Davi Martins Figueiredo - 1037487

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "protocol.h"

void show_menu() {
    printf("\n--- MENU URNA ELETRONICA ---\n");
    printf("1. Listar Candidatos\n");
    printf("2. Votar\n");
    printf("3. Ver Placar Parcial\n");
    printf("4. Sair (BYE)\n");
    printf("9. ADMIN: Fechar Eleicao\n");
    printf("Escolha: ");
}

int main() {
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], server_reply[BUFFER_SIZE];
    int voter_id;

    // Criar socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Erro ao criar socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Conectar
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Conexao falhou");
        return 1;
    }

    printf("Conectado ao servidor!\n");
    
    // Identificação
    printf("Digite seu ID de Eleitor (numero inteiro): ");
    scanf("%d", &voter_id);
    getchar(); // Limpar buffer do teclado

    // Envia HELLO
    snprintf(message, BUFFER_SIZE, "HELLO %d", voter_id);
    if(send(sock, message, strlen(message), 0) < 0) {
        puts("Erro no envio");
        return 1;
    }
    
    // Recebe WELCOME
    if(recv(sock, server_reply, BUFFER_SIZE, 0) < 0) {
        puts("Erro na resposta");
        return 1;
    }
    printf("Servidor: %s\n", server_reply);

    // Loop Principal
    while(1) {
        show_menu();
        int choice;
        scanf("%d", &choice);
        getchar(); // Limpar enter

        memset(message, 0, BUFFER_SIZE);

        if (choice == 1) {
            strcpy(message, CMD_LIST);
        } 
        else if (choice == 2) {
            int vote;
            printf("Digite o numero do candidato: ");
            scanf("%d", &vote);
            getchar();
            snprintf(message, BUFFER_SIZE, "VOTE %d", vote);
        } 
        else if (choice == 3) {
            strcpy(message, CMD_SCORE);
        }
        else if (choice == 4) {
            strcpy(message, CMD_BYE);
            send(sock, message, strlen(message), 0);
            break;
        }
        else if (choice == 9) {
            strcpy(message, CMD_ADMIN_CLOSE);
        }
        else {
            printf("Opcao invalida.\n");
            continue;
        }

        // Envia comando
        if(send(sock, message, strlen(message), 0) < 0) {
            puts("Falha no envio");
            return 1;
        }

        // Recebe resposta
        memset(server_reply, 0, BUFFER_SIZE);
        if(recv(sock, server_reply, BUFFER_SIZE, 0) < 0) {
            puts("Falha na resposta");
            break;
        }

        printf("\n>>> RESPOSTA DO SERVIDOR:\n%s\n", server_reply);
        
        // Se o admin fechou, avisa
        if (strstr(server_reply, "CLOSED FINAL")) {
            printf("\n[INFO] A eleicao foi encerrada pelo Administrador.\n");
        }
    }

    close(sock);
    return 0;
}