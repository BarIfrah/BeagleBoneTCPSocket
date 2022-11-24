#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "Message.h"

//---Defines------------------------------------------------------------------------------------------------------------
const int flattenImageLen = 1024; //(bytes)
const int sharedMemSize = 50;
//---Globals------------------------------------------------------------------------------------------------------------
int tcpSocket, aTrue = 1;
struct sockaddr_in serverAddr, clientAddr;
int imagesInArr = 50;
//---Declarations-------------------------------------------------------------------------------------------------------
void setUpSocket();
int open_shared_memory(key_t);
struct Message * openShmPtr(int shmId);
void terminate(char *);
bool insertImageToShmem(Message *shmemPtr, const char *message);
bool userChooseQuit(char *);
void buildShmemArray(Message *, int);
//----------------------------------------------------------------------------------------------------------------------
int main() {

    int connected;
    long bytesReceived;
    char sendData [flattenImageLen] , recvData[flattenImageLen];
    socklen_t sin_size;


    int shmId;
    key_t key = 5678;     // shmem key (use ftok()?
    struct Message *shmemPtr;
/// Reset struct obj
//    Message m = {.message = "hello", .imageIdentification = "cat", .isConv = false, .isFFT = false, .isReady = false};
//    m = (const struct Message) { 0 };
/// End example

    shmId = open_shared_memory(key);
    shmemPtr = openShmPtr(shmId);
    buildShmemArray(shmemPtr, shmId);
    setUpSocket();

    printf("%lu", sizeof(struct sockaddr));

    sin_size = sizeof(struct sockaddr_in);

    printf("%u", sin_size);

    connected = accept(tcpSocket, (struct sockaddr *)&clientAddr, &sin_size);

    printf("\n I got a connection from (%s , %d)",
           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    while (1)
    {
        printf("\n SEND (q or Q to quit) : ");
        scanf(" %[^\n]s", sendData);

        if (userChooseQuit(sendData)){
            send(connected, sendData, strlen(sendData), 0);
            close(connected);
            break;
        }
        else
            send(connected, sendData, strlen(sendData), 0);

        bytesReceived = recv(connected, recvData, flattenImageLen, 0);

        recvData[bytesReceived] = '\0';

        if (userChooseQuit(recvData)){
            close(connected);
            break;
        }
        else
        {
            printf("\n RECEIVED DATA = %s " , recvData);
            if (!insertImageToShmem(shmemPtr, recvData)){
                /// SHMEM full
                ///TODO send msg & sleep
            } else{
                ///TODO: fft, id, cnlv logic

                ///TODO: remove logic
            }
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
struct Message * openShmPtr(int shmId){
    Message *shmemPtr;
    shmemPtr = (struct Message *) shmat(shmId, NULL, 0);
    if (shmemPtr == (struct Message *) -1) {
        terminate("shmat failed\n");
    }
    return shmemPtr;
}
//----------------------------------------------------------------------------------------------------------------------
/* Gets ptr and char *, builds struct Message object into shared mem.
 * checks if "ready" means that the message that was there was ready and handled earlier. if so - can be deleted.
 *
 */
bool insertImageToShmem(Message *shmemPtr, const char *message){

    for (int i = 0; i < imagesInArr; ++i) {
        if (shmemPtr[i].isReady) {
            ///TODO: func later
            strcpy(shmemPtr[i].message, message);
            shmemPtr[i].isFFT = false;
            shmemPtr[i].isConv = false;
            shmemPtr[i].isReady = false;
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------------------------------------------------------------------
bool userChooseQuit(char *data){
    if (strcmp(data , "q") == 0 || strcmp(data , "Q") == 0)
        return true;
    return false;
}
//----------------------------------------------------------------------------------------------------------------------
// builds the randomized array
void buildShmemArray(Message *shmemPtr, int shmemId){
    for (int i = 0; i < sharedMemSize; ++i) {
        shmemPtr[i] = (const struct Message) { 0 };
        shmemPtr[i].isReady = true;
    }
}