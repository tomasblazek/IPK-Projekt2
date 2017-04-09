/*
 * IPK - Project 2
 * Author: Tomáš Blažek
 */
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>


#define BUFSIZE 1024

int client_socket;
char user[BUFSIZE];

/**
 * Signal Handler close client aplication.
 */
void close_client(int){
    char buffer[BUFSIZE];
    strcpy(buffer, user);
    strcat(buffer, " logged out\r\n");
    send(client_socket, buffer, strlen(buffer), 0);
    close(client_socket);
    exit(0);
}

/**
 *
 * Thread which sending messages to server.
 */
void *send_messages(void *){
    char buffer[BUFSIZE];
    char str[BUFSIZE];
    bzero(buffer,BUFSIZE);
    strcpy(buffer, user);
    strcat(buffer, " logged in\r\n");
    send(client_socket, buffer, strlen(buffer), 0);

    signal(SIGINT, close_client);
    while(1) {
        bzero(buffer,BUFSIZE);
        fgets(str, BUFSIZE, stdin);
        strcpy(buffer, user);
        strcat(buffer, ": ");
        strcat(buffer, str);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    return NULL;
}

/*
 * Thread which recieving messages from server and print them to STDOUT.
 */
void *recev_messages(void *) {
    char buffer[BUFSIZE];
    while(1){
        bzero(buffer, BUFSIZE);
        recv(client_socket, buffer, BUFSIZE, 0);
        printf("%s", buffer);
    }
    return NULL;
}


int main (int argc, const char * argv[]) {
    char server_hostname[1024];
    struct hostent *server;
    struct sockaddr_in server_address;
    int port_number = 21011;

    //Zpracovani argumentu
    if(argc != 5){
        fprintf(stderr,"Error: Bad count of argument %d. (must be 4)\n",argc-1);
        //fprintf(stderr,"usage: %s -r <ROOT-FOLDER> -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int argument;
    while ((argument = getopt(argc, (char* const*) argv, "u:i:")) != -1) {
        switch (argument) {
            case 'i':
                //printf("Server IP: %s\n",optarg);
                strcpy(server_hostname,optarg);
                break;
            case 'u':
                //printf("User: %s\n",optarg);
                strcpy(user, optarg);
                break;
            default:
                fprintf(stderr,"Error: Unknown parameters.\n");
                exit(EXIT_FAILURE);
        }
    }

    // Ziskani adresy serveru pomoci DNS
    if ((server = gethostbyname(server_hostname)) == NULL) {
        fprintf(stderr,"Error: no such host as %s\n", server_hostname);
        exit(EXIT_FAILURE);
    }

    // Nalezeni IP adresy serveru a inicializace struktury server_address
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);

    /* Vytvoreni soketu */
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        perror("Error: socket");
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
        perror("Error: connect");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[2];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if(pthread_create(&threads[0], &attr, recev_messages, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    if(pthread_create(&threads[1], &attr, send_messages, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    while(1){
        sleep(1);
    }
}