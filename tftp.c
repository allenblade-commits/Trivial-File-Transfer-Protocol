/*
 * Name: Allen Stanley
 * Date: 16/04/2026
 * Title: TFTP Common Implementation File
 * Description: Shared source file used by both server and client. Implements
 *              send_file() which reads a file in 512-byte blocks and sends
 *              numbered DATA packets while awaiting ACKs, and receive_file()
 *              which receives DATA packets, writes them to disk, and sends
 *              back ACK responses. Both functions use stop-and-wait flow
 *              control and support octet (binary) transfer mode.
 */

/* Common file for server & client */
#include "tftp.h"
void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename) 
{
    // Implement file sending logic here
    tftp_packet packet;
    tftp_packet ack;
    int bytes_read;
    uint16_t block = 1;
    char mode[8] = "octet";

    //open the file rd mode
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
	    perror("File open failed");
	    return;
    }
    while(1)
    {
	    memset(&packet, 0, sizeof(packet));

	    packet.opcode = htons(DATA);
	    packet.body.data_packet.block_number = htons(block);

	    //check the mode
	    if(strcmp(mode, "octet") == 0)
	    {
		    //read 512 bytes of data from file
		    bytes_read = fread(packet.body.data_packet.data, 1, 512, fp);
	    }
	    else if(strcmp(mode, "netascii") == 0)
	    {
		    bytes_read = fread(packet.body.data_packet.data, 1, 512, fp);
	    }
	    //send the buffer to server
	    sendto(sockfd, &packet, 4 + bytes_read, 0,(struct sockaddr *)&client_addr, client_len);
	    recvfrom(sockfd, &ack, sizeof(uint16_t)+sizeof(uint16_t), 0,(struct sockaddr *)&client_addr, &client_len);

	    //based on ACK send next pack or same pack
	    if(ntohs(ack.body.ack_packet.block_number) == block)
		    block++;

	    if(bytes_read < 512)
		    break;
    }
    fclose(fp);
}

void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename) 
{
    // Implement file receiving logic here
    tftp_packet packet;
    tftp_packet ack;
    int bytes_recv;
    uint16_t expected_block = 1;

    //open the file wr mode
    FILE *fp = fopen(filename, "wb");
    if(fp == NULL)
    {
	    perror("File open failed");
	    return;
    }
    while(1)
    {
	    memset(&packet, 0, sizeof(packet));

	    //recv buffer from server
	    bytes_recv = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &client_len);

	    //check Data packet and block number
	    if(ntohs(packet.opcode) == DATA && ntohs(packet.body.data_packet.block_number) == expected_block)
	    {
		    //write the buffer to file
		    fwrite(packet.body.data_packet.data, 1,bytes_recv - 4, fp);

		    //send the ack
		    memset(&ack, 0,sizeof(ack));
		    ack.opcode = htons(ACK);
		    ack.body.ack_packet.block_number = htons(expected_block);

		    sendto(sockfd, &ack,4, 0, (struct sockaddr *)&client_addr, client_len);
		    expected_block++;

		    //last packet
		    if(bytes_recv < BUFFER_SIZE)
			    break;
	    }
    }
    fclose(fp);
}
