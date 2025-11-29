//Davi Martins Figueiredo - 1037487

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "protocol.h"

// --- ESTRUTURAS E GLOBAIS ---
typedef struct {
    int id;
    char name[50];
    int votes;
} Candidate;

Candidate candidates[MAX_CANDIDATES] = {
    {1, "Luffy (One Piece)", 0},
    {2, "Luke Skywalker (Star Wars)", 0},
    {3, "Goku (Dragon Ball)", 0},
    {4, "Gandalf (Senhor dos Anéis)", 0},
    {5, "Voto Nulo", 0}
};

int voters_record[MAX_VOTERS];
int voters_count = 0;
bool election_active = true;

// Mutexes para exclusão mútua
pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER; // Protege votos e lista de eleitores
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;  // Protege escrita no arquivo de log

// --- FUNÇÕES AUXILIARES ---

// Escreve no log de forma thread-safe
void log_event(const char *msg) {
    pthread_mutex_lock(&log_lock);
    FILE *f = fopen("eleicao.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
    pthread_mutex_unlock(&log_lock);
    printf("[LOG] %s\n", msg);
}

// Gera arquivo final
void save_final_result() {
    pthread_mutex_lock(&data_lock); // Bloqueia para leitura consistente
    FILE *f = fopen("resultado_final.txt", "w");
    if (f) {
        fprintf(f, "--- RESULTADO FINAL DA ELEICAO ---\n");
        for (int i = 0; i < MAX_CANDIDATES; i++) {
            fprintf(f, "%s: %d votos\n", candidates[i].name, candidates[i].votes);
        }
        fprintf(f, "Total de votantes: %d\n", voters_count);
        fclose(f);
    }
    pthread_mutex_unlock(&data_lock);
    log_event("Eleicao encerrada. Resultado salvo.");
}

// Verifica se ID já votou (Supõe que mutex já está travado)
bool has_voted(int voter_id) {
    for (int i = 0; i < voters_count; i++) {
        if (voters_record[i] == voter_id) return true;
    }
    return false;
}

// --- THREAD WORKER ---
void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc); // Libera memória alocada no main

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int read_size;
    int my_voter_id = -1;

    while ( (read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0 ) {
        buffer[read_size] = '\0';
        buffer[strcspn(buffer, "\r\n")] = 0; // Remove newline

        if (strlen(buffer) == 0) continue;

        memset(response, 0, BUFFER_SIZE);

        // 1. HELLO
        if (strncmp(buffer, CMD_HELLO, 5) == 0) {
            sscanf(buffer, "HELLO %d", &my_voter_id);
            snprintf(response, BUFFER_SIZE, "%s %d", RES_WELCOME, my_voter_id);
        }
        // 2. LIST
        else if (strcmp(buffer, CMD_LIST) == 0) {
            strcpy(response, RES_OPTIONS);
            char temp[100];
            for(int i=0; i<MAX_CANDIDATES; i++){
                snprintf(temp, 100, "\n[%d] %s", candidates[i].id, candidates[i].name);
                strcat(response, temp);
            }
        }
        // 3. VOTE
        else if (strncmp(buffer, CMD_VOTE, 4) == 0) {
            if (!election_active) {
                strcpy(response, RES_ERR_CLOSED);
            } else {
                int vote_opt = -1;
                sscanf(buffer, "VOTE %d", &vote_opt);
                
                pthread_mutex_lock(&data_lock); // <--- INICIO REGIÃO CRÍTICA
                
                if (has_voted(my_voter_id)) {
                    strcpy(response, RES_ERR_DUP);
                } else if (vote_opt < 1 || vote_opt > MAX_CANDIDATES) {
                    strcpy(response, RES_ERR_INV);
                } else {
                    // Contabiliza
                    candidates[vote_opt-1].votes++;
                    voters_record[voters_count++] = my_voter_id;
                    
                    snprintf(response, BUFFER_SIZE, "%s %d", RES_VOTED, vote_opt);
                    
                    char log_msg[128];
                    snprintf(log_msg, 128, "Voter ID %d votou na opcao %d", my_voter_id, vote_opt);
                    log_event(log_msg);
                }
                
                pthread_mutex_unlock(&data_lock); // <--- FIM REGIÃO CRÍTICA
            }
        }
        // 4. SCORE
        else if (strcmp(buffer, CMD_SCORE) == 0) {
            pthread_mutex_lock(&data_lock); // Leitura segura
            strcpy(response, RES_SCORE);
            char temp[100];
            for(int i=0; i<MAX_CANDIDATES; i++){
                snprintf(temp, 100, "\n%s: %d", candidates[i].name, candidates[i].votes);
                strcat(response, temp);
            }
            pthread_mutex_unlock(&data_lock);
        }
        // 5. ADMIN CLOSE
        else if (strcmp(buffer, CMD_ADMIN_CLOSE) == 0) {
            election_active = false;
            save_final_result();
            strcpy(response, RES_CLOSED);
        }
        // 6. BYE
        else if (strcmp(buffer, CMD_BYE) == 0) {
            break;
        }
        else {
            strcpy(response, "UNKNOWN COMMAND");
        }

        send(sock, response, strlen(response), 0);
    }

    close(sock);
    return 0;
}

// --- MAIN SERVER ---
int main() {
    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(struct sockaddr_in);

    // Limpa log antigo
    remove("eleicao.log");
    remove("resultado_final.txt");

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) { printf("Erro ao criar socket"); return 1; }

    // Reutilizar porta imediatamente
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro no bind");
        return 1;
    }

    listen(server_sock, 20);
    
    printf("--- SERVIDOR DE VOTACAO INICIADO NA PORTA %d ---\n", PORT);
    log_event("Servidor Iniciado");

    while ((client_sock = accept(server_sock, (struct sockaddr *)&client, &c))) {
        printf("Nova conexão recebida.\n");

        pthread_t sniffer_thread;
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, client_handler, (void*) new_sock) < 0) {
            perror("Erro ao criar thread");
            return 1;
        }
        
        // Detach para thread limpar memória ao terminar sozinha
        pthread_detach(sniffer_thread);
    }

    return 0;
}