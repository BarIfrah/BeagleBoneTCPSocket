
#ifndef SHAREDMEMCLIENT_H
#define SHAREDMEMCLIENT_H



#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#include "Message.h"


void terminate(char *);
int openSharedMemory(key_t);
struct Message *openShmPtr(int );


#endif //SHAREDMEMCLIENT_H
