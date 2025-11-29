//Davi Martins Figueiredo - 1037487

#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PORT 8080
#define BUFFER_SIZE 2048
#define MAX_CANDIDATES 5
#define MAX_VOTERS 100 

// Comandos enviados pelo Cliente
#define CMD_HELLO "HELLO"
#define CMD_LIST "LIST"
#define CMD_VOTE "VOTE"
#define CMD_SCORE "SCORE"
#define CMD_BYE "BYE"
#define CMD_ADMIN_CLOSE "ADMIN CLOSE"

// Respostas do Servidor
#define RES_WELCOME "WELCOME"
#define RES_OPTIONS "OPTIONS"
#define RES_VOTED "OK VOTED"
#define RES_ERR_DUP "ERR DUPLICATE"
#define RES_ERR_INV "ERR INVALID_OPTION"
#define RES_ERR_CLOSED "ERR CLOSED"
#define RES_SCORE "SCORE"
#define RES_CLOSED "CLOSED FINAL"

#endif