#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "Message.h"

//---Defines------------------------------------------------------------------------------------------------------------
const int flattenImageLen = 1000; //(bytes)
const int sharedMemSize = 50;
//---Globals------------------------------------------------------------------------------------------------------------
int tcpSocket, aTrue = 1;
struct sockaddr_in serverAddr, clientAddr;
int imagesInArr = 50;
//---Declarations-------------------------------------------------------------------------------------------------------
void setUpSocket();
int open_shared_memory(key_t);
int *open_shm_ptr(int);
void terminate(char *);
bool insertImageToShmem(int *, Message);
//----------------------------------------------------------------------------------------------------------------------
int main() {

    int connected;
    long bytesRecieved;
    char sendData [1000] , recvData[1000];
    socklen_t sin_size;

    int shmId;
    key_t key = 5678;     // shmem key (use ftok()?
    int *shmemPtr;

    shmId = open_shared_memory(key);
    shmemPtr = open_shm_ptr(shmId);

    setUpSocket();

    sin_size = sizeof(struct sockaddr_in);

    connected = accept(tcpSocket, (struct sockaddr *)&clientAddr, &sin_size);

    printf("\n I got a connection from (%s , %d)",
           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    while (1)
    {
        printf("\n SEND (q or Q to quit) : ");
        gets(sendData);

        if (strcmp(sendData , "q") == 0 || strcmp(sendData , "Q") == 0)
        {
            send(connected, sendData, strlen(sendData), 0);
            close(connected);
            break;
        }
        else
            send(connected, sendData, strlen(sendData), 0);

        bytesRecieved = recv(connected, recvData, 1024, 0);

        recvData[bytesRecieved] = '\0';

        if (strcmp(recvData , "q") == 0 || strcmp(recvData , "Q") == 0)
        {
            close(connected);
            break;
        }
        else
        {
            printf("\n RECIEVED DATA = %s " , recvData);
            fflush(stdout);
        }
    }

    close(tcpSocket);
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------
void setUpSocket(){
    if ((tcpSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Socket");
        exit(1);
    }

    //set the socket options -
    // SO_REUSEADDR - option to allow socket to be be opened on the same port on the machine.
    //                useful when recovering from a crash and the socket was not properly closed -
    //                app can be restarted and it will simply open another socket on the same port
    if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &aTrue, sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);          ///port - will get in argv later
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serverAddr.sin_zero), 8);

    if (bind(tcpSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
    }

    if (listen(tcpSocket, 5) == -1){ //<<<<<<<<<<<<<<<<<<<<<<<<<Blocking untill new connection is accepted <<<<<<<<<<<<<<<
        perror("Listen");
        exit(1);
    }

    printf("\nTCPServer Waiting for client on port 5000");
    fflush(stdout);
}
//----------------------------------------------------------------------------------------------------------------------
//this function terminate the program if there is an error in fork
void terminate(char *error_message){
    perror (error_message);
    exit(EXIT_FAILURE);
}
//----------------------------------------------------------------------------------------------------------------------
// opens the shared memory for the program
int open_shared_memory(key_t key){
    int shm_id;
    shm_id = shmget (key, sharedMemSize * sizeof (Message), IPC_CREAT | IPC_EXCL | 0666) ;
    if (shm_id == -1){
        terminate("shared memory id error\n");
    }
    return shm_id;
}
//----------------------------------------------------------------------------------------------------------------------
// opens and returns a pointer
int *open_shm_ptr(int shm_id){
    int *shmem_ptr;
    shmem_ptr = (int*) shmat(shm_id, NULL, 0);
    if (shmem_ptr == (int*) -1) {
        terminate("shmat failed\n");
    }
    return shmem_ptr;
}
//----------------------------------------------------------------------------------------------------------------------
bool insertImageToShmem(int *shmemPtr, Message message){
    for (int i = 0; i < imagesInArr; ++i) {
        if (shmemPtr[i] == NULL) {
            shmemPtr[i] = message; //malloc?
            return false;
        }
    }
    return true;
}
