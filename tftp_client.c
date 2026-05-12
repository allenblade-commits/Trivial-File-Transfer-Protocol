/*
 * Name: Allen Stanley
 * Date: 16/04/2026
 * Title: TFTP Client Implementation File
 * Description: Implements the TFTP client with a command-line interface.
 *              Supports four commands: connect (initialises UDP socket and
 *              sets server address), get (sends RRQ and downloads a file),
 *              put (sends WRQ and uploads a .txt file), and exit (closes
 *              the socket). Includes IP validation, file existence checks,
 *              and socket timeout configuration.
 */

#include "tftp.h"
#include "tftp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() 
{
    struct sockaddr_in server_addr;	
    char command[256];
    tftp_client_t client;
    memset(&client, 0, sizeof(client));  // Initialize client structure

    // Main loop for command-line interface
    while (1) 
    {
        printf("tftp> ");
        fgets(command, sizeof(command), stdin);

        // Remove newline character
        command[strcspn(command, "\n")] = 0;

        // Process the command
        process_command(&client, command);
    }

    return 0;
}

int validate_ip(char *ip)
{
	struct sockaddr_in sa;

	if(inet_pton(AF_INET, ip, &sa.sin_addr) == 1)
	{
		return 1; //valid
	}
	return 0; //invalid
}

int validate_file(char *filename)
{
    int len = strlen(filename);

    // check minimum length and ".txt" extension
    if(len < 5 || strcmp(filename + len - 4, ".txt") != 0)
    {
        return 0;   // not a .txt file
    }

    // check file existence
    if(access(filename, F_OK) == 0)
    {
        return 1;   // valid .txt file exists
    }

    return 0;       // file does not exist
}


// Function to process commands

void process_command(tftp_client_t *client, char *command)
{
    char cmd[32], arg[128];
    int port;

    int count = sscanf(command, "%s %s %d", cmd, arg, &port);

    if(strcmp(cmd, "connect") == 0)
    {
        if(count != 3)
        {
            printf("Usage: connect <ip> <port>\n");
            return;
        }
        if(!validate_ip(arg))
        {
            printf("Invalid IP address\n");
            return;
        }
        connect_to_server(client, arg, port);
    }
    else if(strcmp(cmd, "put") == 0)
    {
        if(count != 2)
        {
            printf("Usage: put <filename>\n");
            return;
        }
	if(!validate_file(arg))
	{
		printf("invalid filename\n");
		return;
	}
	if(client->sockfd <= 0)
	{
		printf("not connected to server\n");
		return;
	}
        put_file(client, arg);
    }
    else if(strcmp(cmd, "get") == 0)
    {
        if(count != 2)
        {
            printf("Usage: get <filename>\n");
            return;
        }
	if(client->sockfd <= 0)
	{
		printf("not connected to server\n");
		return;
	}
        get_file(client, arg);
    }
    else if(strcmp(cmd, "exit") == 0)
    {
        disconnect(client);
        exit(0);
    }
    else
    {
        printf("Invalid command\n");
    }
}

// This function is to initialize socket with given server IP, no packets sent to server in this function
void connect_to_server(tftp_client_t *client, char *ip, int port) 
{
	struct timeval tv;

        // Create UDP socket
	client->sockfd = socket(AF_INET,SOCK_DGRAM, 0);
	if(client->sockfd < 0)
	{
		perror("socket creation failed");
		return;
	}

        // Set socket timeout option
        tv.tv_sec = TIMEOUT_SEC;
	tv.tv_usec = 0;
	setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        // Set up server address
        client->server_addr.sin_family = AF_INET;
        client->server_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &client->server_addr.sin_addr);

        client->server_len = sizeof(client->server_addr);
        strcpy(client->server_ip, ip);

        printf("Connected to %s:%d\n", ip, port);

}

void put_file(tftp_client_t *client, char *filename)
{
	tftp_packet resp;
	socklen_t len = client->server_len;
        // Send WRQ request and send file
        send_request(client->sockfd, client->server_addr,filename,WRQ);
        if(recvfrom(client->sockfd, &resp, sizeof(resp), 0, (struct sockaddr *)&client->server_addr, &len) < 0)
        {
	       perror("WRQ ACK not received");
	       return;
        }
        if(ntohs(resp.opcode) != ACK)
        {
	       printf("WRQ failed\n");
        }
        else
        {
	       printf("uploading file mode : netascii %s\n",filename);
        }
        //send file data
        send_file(client->sockfd, client->server_addr, client->server_len, filename);
        printf("File uploaded successfully\n");
}

void get_file(tftp_client_t *client, char *filename)
{
    // Send RRQ request
    send_request(client->sockfd, client->server_addr, filename, RRQ);

    printf("Downloading file mode : netascii %s\n", filename);

    // Let receive_file() handle DATA + ACK loop
    receive_file(client->sockfd,client->server_addr,client->server_len,filename);

    printf("File downloaded successfully\n");
}


void disconnect(tftp_client_t *client) 
{
    // close fd
    if(client->sockfd > 0)
    {
	    close(client->sockfd);
	    client->sockfd = 0;
	    printf("disconnect from server\n");
    }
   
}

void send_request(int sockfd, struct sockaddr_in server_addr,char *filename, int opcode)
{
    char buffer[516];
    int len = 0;
    uint16_t op = htons(opcode);

    memcpy(buffer, &op, 2);
    len = 2;

    strcpy(buffer + len, filename);
    len += strlen(filename) + 1;

    strcpy(buffer + len, "octet");
    len += strlen("octet") + 1;

    sendto(sockfd, buffer, len, 0,(struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("send the request\n");
}

void receive_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode)
{
	tftp_packet packet;
	socklen_t server_len = sizeof(server_addr);

	//receive first response from server
	if(recvfrom(sockfd, &packet, sizeof(packet), 0,(struct sockaddr *)&server_addr, &server_len) < 0)
	{
		perror("receive request failed");
		return;
	}

	//RRQ server will send data
	if(opcode == RRQ && ntohs(packet.opcode) == DATA)
	{
		printf("DATA packet received for file: %s\n", filename);
	}
	else if(opcode == WRQ && ntohs(packet.opcode) == ACK)
	{
		printf("ACK received from server for file: %s\n",filename);
	}
}
