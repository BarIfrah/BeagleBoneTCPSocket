
#include "Convolution.h"

//----------------------------------------------------------------------------------------------------------------------
int main()
{
    int shmId;
    key_t key = 5678;     // shmem key (use ftok()?
    struct Message *shmemPtr;

    printf("CNLV started\n");

    shmId = openSharedMemory(key);

    printf("CNLV attached to memory %d\n",shmId);

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
        if (shmPtr[i].isFFT && !shmPtr[i].isConv){
            processImage(&shmPtr[i]);
            sleep(1);
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------------------------------------------------------------------
void processImage(Message *message){
    message->isConv = true;
    printf("CNLV DONE on %s\n", message->message);
}