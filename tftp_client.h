/*
 * Name: Allen Stanley
 * Date: 16/04/2026
 * Title: TFTP Client Header File
 * Description: Defines the tftp_client_t context structure (socket, server
 *              address, IP string) and declares all client function prototypes:
 *              connect_to_server(), put_file(), get_file(), disconnect(),
 *              process_command(), send_request(), and receive_request().
 */

#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<errno.h>

typedef struct 
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len;
    char server_ip[INET_ADDRSTRLEN];
} tftp_client_t;

// Function prototypes
void connect_to_server(tftp_client_t *client, char *ip, int port);
void put_file(tftp_client_t *client, char *filename);
void get_file(tftp_client_t *client, char *filename);
void disconnect(tftp_client_t *client);
void process_command(tftp_client_t *client, char *command);


void send_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode);
void receive_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode);

#endif
