/*
 * Name: Allen Stanley
 * Date: 16/04/2026
 * Title: TFTP Common Header File
 * Description: Shared header included by both server and client. Defines
 *              port, buffer size, timeout constants, the tftp_opcode enum
 *              (RRQ/WRQ/DATA/ACK/ERROR), and the tftp_packet struct with
 *              its union body. Also declares the shared send_file() and
 *              receive_file() function prototypes implemented in tftp.c.
 */

/* Common file for server & client*/

#ifndef TFTP_H
#define TFTP_H

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include<unistd.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>

#define PORT 6969
#define BUFFER_SIZE 516  // TFTP data packet size (512 bytes data + 4 bytes header)
#define TIMEOUT_SEC 5    // Timeout in seconds

// TFTP OpCodes
typedef enum {
    RRQ = 1,  // Read Request
    WRQ = 2,  // Write Request
    DATA = 3, // Data Packet
    ACK = 4,  // Acknowledgment
    ERROR = 5 // Error Packet
} tftp_opcode;

// TFTP Packet Structure
typedef struct {
    uint16_t opcode; // Operation code (RRQ/WRQ/DATA/ACK/ERROR)
    union {
        struct {
            char filename[256];
            char mode[8];  // Typically "octet"
        } request;  // RRQ and WRQ
        struct {
            uint16_t block_number;
            char data[512];
        } data_packet; // DATA
        struct {
            uint16_t block_number;
        } ack_packet; // ACK
        struct {
            uint16_t error_code;
            char error_msg[512];
        } error_packet; // ERROR
    } body;
} tftp_packet;

void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename);
void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename);


#endif // TFTP_H
