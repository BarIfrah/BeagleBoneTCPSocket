
#include <stdbool.h>

typedef struct Message{
    char message[1000];
    bool isReady;
    bool isConv;
    bool isFFT;
    bool canWorkOn;
    char imageIdentification[20];
}Message;

