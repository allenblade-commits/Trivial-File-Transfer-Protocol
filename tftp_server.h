/*
 * Name: Allen Stanley
 * Date: 16/04/2026
 * Title: TFTP Server Header File
 * Description: Defines constants, packet structures, and function prototypes
 *              specific to the TFTP server. Declares handle_client() for
 *              dispatching incoming RRQ/WRQ requests and send_file() for
 *              transmitting file data in 512-byte blocks over UDP.
 */

#ifndef TFTP_H
#define TFTP_H

/* ---------- STANDARD HEADERS ---------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>

#define PORT 6969
#define BUFFER_SIZE 516

#define RRQ   1
#define WRQ   2
#define DATA  3
#define ACK   4
#define ERROR 5

typedef struct
{
    uint16_t opcode;

    union
    {
        struct
        {
            char filename[256];
            char mode[12];      /* "octet" or "netascii" */
        } request;

        struct
        {
            uint16_t block_number;
            char data[512];
        } data_packet;

        struct
        {
            uint16_t block_number;
        } ack_packet;

        struct
        {
            uint16_t error_code;
            char error_msg[512];
        } error_packet;

    } body;

} tftp_packet;


void handle_client(int sockfd,struct sockaddr_in client_addr,socklen_t client_len,tftp_packet *packet);

//void receive_file(int sockfd,struct sockaddr_in client_addr,socklen_t client_len,char *filename);

void send_file(int sockfd,struct sockaddr_in client_addr,socklen_t client_len,char *filename);

#endif 
