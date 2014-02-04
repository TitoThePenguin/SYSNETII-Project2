/*
 * TCPclient.c
 * Systems and Networks II
 * Project 1
 *
 * This file describes the functions to be implemented by the TCPclient.
 * You may also implement any auxillary functions you deem necessary.
 */
 
#include <stdio.h> 			/* for printf() and fprintf() */
#include <sys/socket.h>			/* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>			/* for sockaddr_in and inet_addr() */
#include <stdlib.h>			/* for atoi() and exit() */
#include <string.h>			/* for memset() */
#include <unistd.h>			/* for close() */
#include <netinet/in.h>
#include <netdb.h>
#include "TCPclient.h"

#define RCVBUFSIZE 256   /* Size of receive buffer */

/*
 * Creates a streaming socket and connects to a server.
 *
 * serverName - the ip address or hostname of the server given as a string
 * port       - the port number of the server
 * dest       - the server's address information; the structure should be created with information
 *              on the server (like port, address, and family) in this function call
 *
 * return value - the socket identifier or a negative number indicating the error if a connection could not be established
 */
int createSocket(char * serverName, int port, struct sockaddr_in * dest)
{
		int socketCreation = 0, temp, temp2, temp3;
		memset(dest, '0', sizeof(struct sockaddr_in));
		dest->sin_family = AF_INET;
		dest->sin_port = htons(port);
		temp2 = inet_pton(AF_INET, serverName, &dest->sin_addr); //Sever IP address
		if(temp2 <= 0)
		{
			fprintf(stderr, "\ninet_pton error\n");
			exit(1);
		}
		socketCreation = socket(PF_INET, SOCK_DGRAM, 0);
		//printf("SocketCreation is %d\n", socketCreation);
		return socketCreation;
}

/*
 * Sends a request for service to the server. This is an asynchronous call to the server, 
 * so do not wait for a reply in this function.
 * 
 * sock    - the socket identifier
 * request - the request to be sent encoded as a string
 * dest    - the server's address information
 *
 * return   - 0, if no error; otherwise, a negative number indicating the error
 */
int sendRequest(int sock, char * request, struct sockaddr_in * dest)
{
	printf("Sending request\n");
	int error = 0;
	error = send(sock, request, strlen(request), 0);
	return error;
}	

/*
 * Receives the server's response formatted as an XML text string.
 *
 * sock     - the socket identifier
 * response - the server's response as an XML formatted string to be filled in by this function;
 *            memory is allocated for storing response
 *
 * return   - 0, if no error; otherwise, a negative number indicating the error
 */
int receiveResponse(int sock, char * response)
{
	int error = 0;
	error = recv(sock, response, RCVBUFSIZE, 0);
	return error;
}

/*
 * Prints the response to the screen in a formatted way.
 *
 * response - the server's response as an XML formatted string
 *
 */
void printResponse(char* response)
{
	printf("\nResponse is: %s\n", response);
	//showASCII(response);
}

/*
 * Closes the specified socket
 *
 * sock - the ID of the socket to be closed
 * 
 * return - 0, if no error; otherwise, a negative number indicating the error
 */
int closeSocket(int sock)
{
	int error = 0;
	error = close(sock);
	return error;
}



void emptyBuffer(char * arry, int sizeArry)		//A utility function that empty a buffer of a string
{
	int i;
	for(i = 0; i < sizeArry; i++)
	{
		arry[i] = '\0';	
	}	
}

void showASCII(char * str)			//A utility function that shows the ascii number for a string
{
    int i, sentSize, j;
    sentSize = strlen(str);
    sentSize = sentSize + 3;
    for(i = 0; i < sentSize; i++)
    {
      	printf("%d ", str[i]);   
    }
    printf("\n");
}

void printArguments(int argc, char ** argv)			//A utility function that shows the arguments entered into the program
{
	printf("Argurment 1 is: %s\n", argv[0]);
	printf("Argurment 2 is: %s\n", argv[1]);
	printf("Argument 3 is: %s\n", argv[2]);

}
