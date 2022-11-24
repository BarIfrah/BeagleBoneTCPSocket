#include "ImageIdentification.h"

//----------------------------------------------------------------------------------------------------------------------
int main()
{
    int shmId;
    key_t key = 5678;     // shmem key (use ftok()?
    struct Message *shmemPtr;

    printf("IM ID started\n");

    shmId = openSharedMemory(key);

    printf("IM ID attached to memory %d\n",shmId);

    shmemPtr = openShmPtr(shmId);
    while (true) {
        locateImageToProcess(shmemPtr);
//        if(!locateImageToProcess(shmemPtr)){
//            break;
//        };
    }

    printf("Client exits.\n");

    return 0;
}
//----------------------------------------------------------------------------------------------------------------------
bool locateImageToProcess(Message *shmPtr){
    for (int i = 0; i < 50; ++i) {
        if (shmPtr[i].isConv && shmPtr[i].isFFT && !shmPtr[i].isReady){
            processImage(&shmPtr[i]);
            sleep(1);
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------------------------------------------------------------------
void processImage(Message *message){
    strcpy(message->imageIdentification, "cat");
    message->isReady = true;
    printf("ID DONE, %s is %s\n", message->message, message->imageIdentification);
}