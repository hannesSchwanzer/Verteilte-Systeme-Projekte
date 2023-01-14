/**
 * @file	hamster_server.c
 * @brief	Remote Hamsterverwaltungsprogramm
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/hamsterlib.h"

#include "hamsterprotocol.h"
#include "server_util.h"
#include "request_handler.h"

/** Default port number and IP */
#define DEFAULT_PORT 2323
#define DEFAULT_IP   "127.0.0.1"

/**
 * @brief Kurze Hilfe ausgeben
 */
static void rtfm(char* argv[])
{
	printf("Usage: %s {<Option>} <param1> \n", argv[0]);
	printf("Function: Hamster management server\n");
	printf("Optionen:\n");
	printf("     -p {<port>}                    - port to run the server\n");
	printf("     -h {<IP address>}              - IP address to run the server on (default: 127.0.0.1\n");
}

/**
 * @brief Main program
 */
int main(int argc, char* argv[])
{
	unsigned long  port = DEFAULT_PORT;
	char *ipaddr = DEFAULT_IP;
	char option;
	char *end;
	// parse arguments
	while ((option = getopt(argc, argv, "?h:p:v")) != -1)
	{
		switch(option)
		{
			case '?':
				rtfm(argv);
				return EXIT_SUCCESS;
				break;
			case 'p':
				port = strtoul(optarg, &end, 0);
				if (end == optarg)
				{
					rtfm(argv);
					exit(EXIT_FAILURE);
				}
				break;
			case 'h':
				ipaddr = optarg;
				break;
		}
	}
	if(argc - optind)
	{
		rtfm(argv);
		return 0;
	}

    //Main functionality for the server

	printf("Starting Server on Port %lu\n", port);

    //creating a socket
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    //setting up the socket addres
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(port);
	myaddr.sin_addr.s_addr = inet_addr(ipaddr);

    if (myaddr.sin_addr.s_addr == (in_addr_t) -1) {
        printf("IP adress couldn't be read\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }


    //binding the socket with the address
    if (bind(sock_fd, (struct sockaddr*) &myaddr, sizeof(myaddr)) == -1) {
        perror("bind()");
	    close(sock_fd);
        exit(EXIT_FAILURE);
    }

    //listening (5 clients max)
    if (listen(sock_fd, 5) == -1) {
        perror("listen()");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
	printf("Listening...\n");

    //Main loop
	while (true)
	{
        //accept new connection
		struct sockaddr clientaddr;
		socklen_t clientaddrlength = sizeof(clientaddr);
		int connfd = accept(sock_fd, &clientaddr, &clientaddrlength);
        if (connfd == -1) {
            perror("accept()");
            break;
        }

		while (true) 
		{
            //handle requests until client closes the connection
			struct header header;
			if (read_header(connfd, &header) == -1) break;

			print_header(&header);
			if ((header.version & 0xff) != 0xb6) {
				break;
			}
			char* buf = malloc(header.payload_length+1);
			readFromSocket(connfd, buf, header.payload_length);
			dump((unsigned char*) buf, header.payload_length);
			handle_request(connfd, &header, buf);
			free(buf);
		}
			
		close(connfd);
	}
	
	close(sock_fd);
}

