/*
 * Name: Allen Stanley
 * Date: 16/04/2026
 * Title: TFTP Server Implementation File
 * Description: Implements the TFTP server main loop. Creates a UDP socket,
 *              binds to port 6969, and continuously listens for incoming
 *              packets. Dispatches each request to handle_client() which
 *              handles WRQ (PUT) by sending ACK block 0 then calling
 *              receive_file(), and RRQ (GET) by calling send_file().
 */

#include "tftp.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>

/* Function prototype */
void handle_client(int sockfd,struct sockaddr_in client_addr,socklen_t client_len,tftp_packet *packet);

int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    tftp_packet packet;

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    //TODO Use setsockopt() to set timeout option

    /* Server address setup */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    /* Bind */
    if(bind(sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    printf("TFTP Server started on port %d\n", PORT);

    /* Main server loop */
    while(1)
    {
        client_len = sizeof(client_addr);

        int n = recvfrom(sockfd,&packet,sizeof(packet),0,(struct sockaddr *)&client_addr,&client_len);
        if (n < 0)
        {
            perror("recvfrom");
            continue;
        }

        /* Print client details */
        printf("Request received from %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        handle_client(sockfd, client_addr, client_len, &packet);
    }
}

/* Handle RRQ / WRQ */
void handle_client(int sockfd,struct sockaddr_in client_addr,socklen_t client_len,tftp_packet *packet)
{
    uint16_t opcode = ntohs(packet->opcode);
    tftp_packet ack;
    
    /* WRITE REQUEST (PUT) */
    if(opcode == WRQ)
    {
        FILE *fp = fopen(packet->body.request.filename, "wb");
        if(!fp)
        {
            perror("file create");
            return;
        }
        fclose(fp);

        /* Send ACK block 0 */
        memset(&ack, 0, sizeof(ack));
        ack.opcode = htons(ACK);
        ack.body.ack_packet.block_number = htons(0);

        sendto(sockfd,&ack,4,0,(struct sockaddr *)&client_addr,client_len);

        printf("PUT request for file: %s\n",packet->body.request.filename);

        receive_file(sockfd,client_addr,client_len,packet->body.request.filename);

        printf("File transfer completed\n");
    }
    else if(opcode == RRQ)
    {
        FILE *fp = fopen(packet->body.request.filename, "rb");
        if (!fp)
        {
            perror("file not found");
            return;
        }
        fclose(fp);

        printf("GET request for file: %s\n",packet->body.request.filename);

        send_file(sockfd,client_addr,client_len,packet->body.request.filename);

        printf("File transfer completed\n");
    }
    
}
