# Sistema de Votação Distribuído em Tempo Real

## Visão Geral
Este projeto é uma implementação de um sistema de votação eletrônica baseado na arquitetura **Cliente-Servidor**. O sistema foi desenvolvido em C para a disciplina de Computação Distribuída, demonstrando o uso de **Sockets TCP** para comunicação em rede e **Threads POSIX** para concorrência.

O servidor é capaz de lidar com múltiplos clientes simultaneamente, garantindo a integridade dos dados (como contagem de votos e lista de eleitores) através do uso de **Mutexes** (Exclusão Mútua).

---

## Tecnologias Utilizadas
* **Linguagem:** C
* **Comunicação:** Sockets TCP/IP
* **Concorrência:** POSIX Threads (pthread)
* **Sincronização:** Mutex
* **Ambiente:** Linux (Ubuntu)

---

## Estrutura de Arquivos

* **server.c**: Código fonte do servidor. Gerencia conexões, threads de trabalhadores e lógica de votação.
* **client.c**: Código fonte do cliente. Interface de terminal para o eleitor.
* **protocol.h**: Arquivo de cabeçalho contendo configurações (Porta 8080) e constantes do protocolo de comunicação.
* **Makefile**: Script para automatizar a compilação.
* **eleicao.log**: Arquivo de auditoria gerado automaticamente pelo servidor (Logs).
* **resultado_final.txt**: Arquivo gerado ao encerrar a eleição com o placar final.

---

## Compilação

Para compilar o projeto, certifique-se de estar no diretório raiz dos arquivos e execute o comando `make`. O Makefile cuidará de compilar o servidor e o cliente, linkando as bibliotecas necessárias (`-pthread`).

Comando de compilação:

```bash
make
```
Para limpar os arquivos executáveis e logs antigos:
```bash
make clean
```

---

## Execução

Para simular o ambiente distribuído, você deve executar o servidor em um terminal e os clientes em outros terminais separados.

### 1. Iniciando o Servidor
Abra um terminal e execute:
```bash
./server
```


*O servidor iniciará na porta 8080 e aguardará conexões.*

### 2. Iniciando um Cliente (Eleitor)
Abra um novo terminal (ou nova aba) e execute:
```bash
./client
```

---

## Guia de Uso (Cliente)

Ao iniciar o cliente, siga os passos:

1.  **Identificação:** Digite um ID numérico (ex: 101, 202).
    * *Nota:* O sistema bloqueia múltiplos votos do mesmo ID.

2.  **Menu Principal:**
    * **1. Listar Candidatos:** Mostra as opções de voto disponíveis.
    * **2. Votar:** Solicita o número do candidato. Se o ID já tiver votado, o servidor retornará `ERR DUPLICATE`.
    * **3. Ver Placar Parcial:** Solicita ao servidor a contagem atual dos votos.
    * **4. Sair (BYE):** Desconecta o cliente, mas mantém o servidor rodando.
    * **9. ADMIN: Fechar Eleição:** Comando administrativo que encerra a votação no servidor e gera o relatório final.

---

## Arquivos de Saída

### Log de Eventos (`eleicao.log`)
Durante a execução, o servidor registra atividades importantes neste arquivo para fins de auditoria. Exemplo de conteúdo:
```bash
Servidor Iniciado 
Voter ID 100 votou na opcao 3 
Voter ID 101 votou na opcao 1 
Eleicao encerrada. Resultado salvo.
```
### Resultado Final (`resultado_final.txt`)
Gerado apenas quando um usuário envia o comando **ADMIN CLOSE** (Opção 9). Contém o somatório final dos votos.

```bash
--- RESULTADO FINAL DA ELEICAO ---
 Luffy (One Piece): 1 votos 
 Luke Skywalker (Star Wars): 0 votos 
 Goku (Dragon Ball): 1 votos 
 Gandalf (Senhor dos Anéis): 0 votos 
 Voto Nulo: 0 votos 
 Total de votantes: 2
```

## Solução de Problemas

**Erro: "Address already in use"**
Se você tentar iniciar o `./server` e receber este erro, significa que a porta 8080 ainda está ocupada (provavelmente uma execução anterior que não foi fechada corretamente).
* *Solução:* Aguarde alguns segundos ou mate o processo anterior.

**Erro: "ERR DUPLICATE" ao votar**
Isso é um comportamento esperado de segurança. O sistema impede que o mesmo ID vote mais de uma vez.
* *Solução:* Para testar novamente, use um ID diferente ou reinicie o servidor para limpar a memória.