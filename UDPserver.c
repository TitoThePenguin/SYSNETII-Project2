#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <stdint.h>

#define _GNU_SOURCE
#include <string.h>

#define BUFFER_SIZE 256
#define MAX_NUM_LISTENER_ALLOWED 10

/*
 * Identifies if the specified message is well-formed or not.
 *
 * msg - the message to be tested
 *
 * returns a value greater than 0 if the message is well formed and a value less than zero if it is not
 */
int wellFormed(char* msg)
{
        int length = strlen(msg);

        if (strcasecmp(msg, "<loadavg/>") == 0) {
                if (length != 10) {
                        return -1;
                }
                return 1;
        }
        if (strncasecmp(msg, "<echo>", 6) == 0) {
                char* endStr = strstr(msg, "</echo>");
                if (endStr == NULL)
                        return -1;
                if (endStr[7] != '\0')
                        return -1;
                return 1;
        }

        return -1;
}

/*
 * Determines the load average and writes it as an XML formatted string into the specified string location.
 *
 * resp - memory for a text string to store the XML formatted string specifying load average information
 */
void getLoadAvgResp(char* resp)
{
        double resultRuntime[3];

        bzero(resp, BUFFER_SIZE);

        if (getloadavg(resultRuntime, 3) < 0) {
                strcpy(resp, "<error>unable to obtain load average</error>");
                return;
        }

        sprintf(resp, "<replyLoadAvg>%f:%f:%f</replyLoadAvg>", resultRuntime[0], resultRuntime[1], resultRuntime[2]);
}

/*
 * Generates a response to a request message in accordance with the given protocol in the project documentation.
 * The request message is assumed to be in a correct format.
 *
 * msg - the message that specifies the request from a client
 * resp - memory for a text string to store the response to a request
 */
void getResponse(char* msg, char* resp)
{
        bzero(resp, BUFFER_SIZE);

        if (strcasecmp(msg, "<loadavg/>") == 0) {
                getLoadAvgResp(resp);
                return;
        }

        if (strncasecmp(msg, "<echo>", 6) == 0) {
                int payLoadLength = strlen(msg)-13;
                strcpy(resp, "<reply>");
                strncpy(&resp[7], &msg[6], payLoadLength);
                strcpy(&resp[7+payLoadLength], "</reply>");
                return;
        }

        if(strcasecmp(msg, "<shutdown/>") == 0)
        {
                fprintf(stderr, "Closing socket & terminating...\n");

                exit(0);
        }
}

/*
 * Writes an error message into the specified character array.
 *
 * msg - memory for a text string to store the error message
 */
void createError(char* msg)
{
        bzero(msg, BUFFER_SIZE);
        strcpy(msg, "<error>unknown format</error>");
}

/*
 * Prints the host name of the machine to the screen and returns the port #.
 */
int printHostInfo(int argc, char ** argv)
{
        int portNum = -1;
        if(argc == 2)
                portNum = atoi(argv[1]);
        else
        {
                fprintf(stderr, "Invalid number of argumnets! Need 1 port #\n");
                exit(0);
        }
        // getting host name and use static port binding
        char hostname[128];
        gethostname(hostname, 128);
        fprintf (stderr, "Hostname: %s\n", hostname);

        return portNum;
}

/*
 * Creates a socket and binds it statically. Port information of the bound socket is returned.
 */
int createSocket(int portNum)
{
        struct sockaddr_in servaddr;
        int                listensockfd;

        // creating a streaming server socket
        listensockfd = socket (AF_INET, SOCK_DGRAM, 0);
        if (listensockfd < 0) {
                return -1;
        }

        // build local address and set port to unknown
        memset ((void *) &servaddr, 0, (size_t)sizeof(servaddr));
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(portNum);

        // bind statically
        bind (listensockfd, (struct sockaddr *) &servaddr, (socklen_t)sizeof(servaddr));

        // retrieve port information
        struct sockaddr_in boundAddr;
        socklen_t          sizeAddr = sizeof(boundAddr);
        getsockname(listensockfd, (struct sockaddr *) &boundAddr, &sizeAddr);
        fprintf(stderr, "Port #: %d\n", ntohs(boundAddr.sin_port));

        return listensockfd;
}

/*
 * Implements the request handler executed within a thread.
 */
void reqHandler(int sockValue, char * message, int recLength, struct sockaddr_in cliAddr, int cliLength)
{
        int  sockfd = sockValue;
        char resp[BUFFER_SIZE];

        if (wellFormed(message) < 0) {
                createError(resp);
        }
        else {
                getResponse(message, resp);
        }
        if(sendto(sockfd, resp, recLength, 0, (struct sockaddr *)&cliAddr, cliLength) < 0)
                fprintf(stderr, "Could not send response!\n");
        fprintf(stderr, "Sent response %s\n", resp);

        close (sockfd);
}

/*
 * The main program that starts up the server.
 * No parameters are processed.
 */
int main(int argc, char** argv)
{
        int listensockfd, cliLength, recLength;
        struct sockaddr_in cliAddr;
        char message[256];

        //Provides host inforation on screen and creates a datagram server socket
        listensockfd = createSocket(printHostInfo(argc, argv));
        if (listensockfd < 0) {
                perror ("cannot create socket");
                exit (1);
        }
        while(1)
        {
fprintf(stderr, "Starting... ");
                cliLength = sizeof(cliAddr);
                recLength = recvfrom(listensockfd, message, 256, 0, (struct sockaddr *)&cliAddr, (socklen_t *)&cliLength);
                message[recLength] = '\0';
fprintf(stderr, "recieved, ... ");
                reqHandler(listensockfd, message, recLength, cliAddr, cliLength);
fprintf(stderr, "done!\n");
        }
}
