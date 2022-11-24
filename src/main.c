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

#include <sys/wait.h>   /* Wait for Process Termination */
#include <termios.h>

#include "Message.h"

//---Defines------------------------------------------------------------------------------------------------------------
/// UART Usage
#define SERIAL_DEVFILE_1 "/dev/ttyS1"
#define SERIAL_DEVFILE_2 "/dev/ttyS2"


const int flattenImageLen = 1024; //(bytes)
const int sharedMemSize = 50;
//---Globals------------------------------------------------------------------------------------------------------------
int tcpSocket, aTrue = 1;
struct sockaddr_in serverAddr, clientAddr;

int uartTcpSocket, uartTrue = 1;
struct sockaddr_in uartServerAddr, uartClientAddr;
int imagesInArr = 50;
//

void setUpSocket();
int openSharedMemory(key_t);
struct Message * openShmPtr(int shmId);
void terminate(char *);
bool insertImageToShmem(Message *shmemPtr, const char *message);
bool userChooseQuit(char *);
void buildShmemArray(Message *shmemPtr);

bool sendToUart(char *);
void configSerial(int );
int sendMessageOverUart(char *);
//----------------------------------------------------------------------------------------------------------------------
int main() {

    int connected;
    long bytesReceived;
    char sendData [flattenImageLen] , recvData[flattenImageLen];
    socklen_t sin_size;


    int shmId;
    key_t key = 5678;     // shmem key (use ftok()?
    struct Message *shmemPtr;
    ///set shared mem
    shmId = openSharedMemory(key);
    shmemPtr = openShmPtr(shmId);
    buildShmemArray(shmemPtr);

    setUpSocket();

    sin_size = sizeof(struct sockaddr_in);

    connected = accept(tcpSocket, (struct sockaddr *)&clientAddr, &sin_size);

    printf("\n I got a connection from (%s , %d)",
           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    strcpy(sendData, "Hello from server\n");
    send(connected, sendData, strlen(sendData), 0);
    sleep(1);
    while (1) {
        send(connected, "insert image as 1Kb str:\n", strlen("insert image as 1Kb str:\n"), 0);
//        printf("\n SEND (q or Q to quit) : ");
//        scanf(" %[^\n]s", sendData);

//        if (userChooseQuit(sendData)){
//            send(connected, sendData, strlen(sendData), 0);
//            close(connected);
//            break;
//        }

        bytesReceived = recv(connected, recvData, flattenImageLen, 0);

        recvData[bytesReceived] = '\0';

        if (userChooseQuit(recvData)){
            close(connected);
            break;
        }
        else
        {
            printf("\n RECEIVED DATA = %s\n" , recvData);
            if (insertImageToShmem(shmemPtr, recvData)){
                printf("inserted img to shmem\n");
//                sleep(5);
            } else{

                ///TODO: fft, id, cnlv logic
                for (int i = 0; i < sharedMemSize; ++i) {
                    if (shmemPtr[i].isReady){
                        if (sendToUart(shmemPtr[i].message)) {
                            shmemPtr[i] = (const struct Message) {0};
                            printf("going to sleep\n");
                            sleep(1);
                            break;
                        }
                    }
                }
            }
            fflush(stdout);
        }
    }
    ///TODO: close shared memory
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
int openSharedMemory(key_t key){
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

    for (int i = 0; i < sharedMemSize; ++i) {
        if (shmemPtr[i].isReady) {
            ///TODO: func later
            strcpy(shmemPtr[i].message, message);
            shmemPtr[i].isFFT = false;
            shmemPtr[i].isConv = false;
            shmemPtr[i].isReady = false;
            shmemPtr[i].canWorkOn = true;
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
void buildShmemArray(Message *shmemPtr) {
    for (int i = 0; i < sharedMemSize; ++i) {
        shmemPtr[i] = (const struct Message) { 0 };
        shmemPtr[i].isReady = true;
    }
}
//----------------------------------------------------------------------------------------------------------------------
//---UART Logic---------------------------------------------------------------------------------------------------------
bool sendToUart(char *message){
    sendMessageOverUart(message);
    return true;
}
//----------------------------------------------------------------------------------------------------------------------
int sendMessageOverUart(char *message){
    int ret, fd_serial1,fd_serial2;
    struct termios options;
    int n;
    char tx_buff[255];
    char rx_buff[255];

    //configure the pins for UART
    printf("we need to configure the pins to UART\n");
    printf("config-pin p9-22 uart\n");
    printf("config-pin p9-24 uart\n");

    // initialize serial connection
    printf(" initialize serial file descriptors\n");

    /* Open Ports */
    fd_serial1 = open(SERIAL_DEVFILE_1, O_RDWR | O_NOCTTY | O_NDELAY);  /* <--- serial port 1 */
    if(fd_serial1 == -1) {
        printf("ERROR Open Serial Port 1!");
        exit(-1);
    }

    fd_serial2 = open(SERIAL_DEVFILE_2, O_RDWR | O_NOCTTY | O_NDELAY);  /* <--- serial port 2 */
    if(fd_serial1 == -1) {
        printf("ERROR Open Serial Port 2!");
        exit(-1);
    }

    // Serial Configuration
    configSerial(fd_serial1);
    configSerial(fd_serial2);

    //writing tx-buffer to serial port
    strcpy(tx_buff,"YOUR COMMAND STRING HERE \n");
    ret = write(fd_serial1,tx_buff,strlen(tx_buff) );
    if (ret == -1) {
        perror("Error writing to device");
        exit(EXIT_FAILURE);
    }

    sleep(1); //wait for HW to write to device

    //reading serial port into rx-buffer
    ret = read(fd_serial2,rx_buff,strlen(tx_buff) );
    if (ret == -1) {
        perror("Error writing to device");
        exit(EXIT_FAILURE);
    }
    printf("ret=%d %s \n",ret,rx_buff);
    close(fd_serial1); // Close Port
    close(fd_serial2); // Close Port
}
//----------------------------------------------------------------------------------------------------------------------
void configSerial(int fd_serial){
    struct termios options;

    tcgetattr(fd_serial, &options);   // Get Current Config
    cfsetispeed(&options, B9600);     // Set Baud Rate
    cfsetospeed(&options, B9600);
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_iflag =  IGNBRK;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cflag |= CLOCAL | CREAD;
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 5;
    options.c_iflag &= ~(IXON|IXOFF|IXANY);
    options.c_cflag &= ~(PARENB | PARODD);

    /* Save The Configuration */
    tcsetattr(fd_serial, TCSANOW, &options);
    /* Flush the input (read) buffer */
    tcflush(fd_serial,TCIOFLUSH);
}
//----------------------------------------------------------------------------------------------------------------------