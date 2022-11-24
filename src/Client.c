
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


//---Defines------------------------------------------------------------------------------------------------------------
const int flattenImageLen1 = 1024; //(bytes)
//---Globals------------------------------------------------------------------------------------------------------------
struct hostent *host;
struct sockaddr_in server_addr;
int tcpSocket;
//---Declarations-------------------------------------------------------------------------------------------------------
void connectToSocket();
//----------------------------------------------------------------------------------------------------------------------
int main(){
    long bytesReceived;
    char sendData[flattenImageLen1], recvData[flattenImageLen1];

    connectToSocket();

    bytesReceived = recv(tcpSocket, recvData, flattenImageLen1, 0);
    recvData[bytesReceived] = '\0';
    printf("\nReceived data = %s " , recvData);

    while(1){
        bytesReceived = recv(tcpSocket, recvData, flattenImageLen1, 0);
        recvData[bytesReceived] = '\0';
        printf("\nReceived data = %s " , recvData);

//        if (strcmp(recvData , "q") == 0 || strcmp(recvData , "Q") == 0){
//            close(tcpSocket);
//            break;
//        } else{
//        }

        printf("\nSEND (q or Q to quit) : ");
        scanf(" %[^\n]s", sendData);

        if (strcmp(sendData , "q") != 0 && strcmp(sendData , "Q") != 0){
            send(tcpSocket, sendData, strlen(sendData), 0);
        }else{
            send(tcpSocket, sendData, strlen(sendData), 0);
            close(tcpSocket);
            break;
        }
        sleep(1); ///sleep 1s after sending image
//        bytesReceived = recv(tcpSocket, recvData, flattenImageLen1, 0);
//        recvData[bytesReceived] = '\0';
//        printf("\nReceived data = %s " , recvData);
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------
void connectToSocket(){
    host = gethostbyname("127.0.0.1");   ///TODO: get in agrv[]

    if ((tcpSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8);

    if (connect(tcpSocket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Connect");
        exit(1);
    }
}
//----------------------------------------------------------------------------------------------------------------------
