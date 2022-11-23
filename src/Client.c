/*
lesson 12-2
file name: domain_tcp_socket_client,
this file is part of a program that illustrates the usage of Connection Oriented Domain Sockets

 Description
 -----------
 The program requires 2 files:
 domain_tcp_socket_server
 domain_tcp_socket_client (THIS ONE)

 the Client side creates a Domain socket and starts wrting to the server
 loop-back (127.0.0.1) is used for Internet Addressing
 the server part should run before the client



To compile me for Linux, use gcc -ggdb domain_tcp_socket_client.c -o tcp_socket_client
To execute, type:  ./tcp_socket_client
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


int main()

{

    int sock, bytes_recieved;
    char send_data[1024],recv_data[1024];
    struct hostent *host;
    struct sockaddr_in server_addr;

    host = gethostbyname("127.0.0.1");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1)
    {
        perror("Connect");
        exit(1);
    }

    while(1)
    {

        bytes_recieved=recv(sock,recv_data,1024,0);
        recv_data[bytes_recieved] = '\0';

        if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0)
        {
            close(sock);
            break;
        }
        else
        {
            printf("\nRecieved data = %s " , recv_data);
        }

        printf("\nSEND (q or Q to quit) : ");
        gets(send_data);

        if (strcmp(send_data , "q") != 0 && strcmp(send_data , "Q") != 0)
        {
            send(sock,send_data,strlen(send_data), 0);
        }
        else
        {
            send(sock,send_data,strlen(send_data), 0);
            close(sock);
            break;
        }

    }
    return 0;
}
