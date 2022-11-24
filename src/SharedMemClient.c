#include "SharedMemClient.h"
//----------------------------------------------------------------------------------------------------------------------
int openSharedMemory(key_t key){
    int shmId;
    if ((shmId = shmget(key, 50 * sizeof(Message), 0666)) < 0) {   /// no shmem in this aera
        terminate("shmget error");
        exit(1);
    }
    return shmId;
}
//----------------------------------------------------------------------------------------------------------------------
//this function terminate the program if there is an error in fork
void terminate(char *errorMessage){
    perror (errorMessage);
    exit(EXIT_FAILURE);
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

