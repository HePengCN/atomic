#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "xqueue.h"

#define COUNT (10000)

void* read_thd(void* hQueue)
{

    char pData[128];
    memset(pData, 0, sizeof(pData));
    uint32_t data_size = 0;
    int ret;

    if(xqueue_empty(hQueue)) {
        printf("queue empty.\n");
    }

    if(xqueue_try_front(hQueue, &pData, &data_size)) {
        printf("try front true\n");
    } else {
        printf("try front false\n");
    }

    for(int i =0; i < COUNT; i++) {
        ret = xqueue_wait_and_front(hQueue, pData, &data_size);
        if(0 != ret) {
            printf("xqueue_wait_and_front return error.\n");
            return hQueue;
        }

        printf("%d: size: %u, pData: %s, queue_size: %u\n", i, data_size, pData, xqueue_size(hQueue));
        //xqueue_free(pData);

        ret = xqueue_wait_and_pop(hQueue, pData, &data_size);
        if(0 != ret) {
            printf("xqueue_wait_and_pop return error.\n");
            return hQueue;
        }

        printf("%d, size: %u, pData: %s, queue_size: %u\n", i, data_size, pData, xqueue_size(hQueue));
        //xqueue_free(pData);
    }

    return hQueue;
}

int main(int argc, char* argv[])
{
    pthread_t hThd;
    void* hQueue;
    hQueue = xqueue_new(128);
    if(0 != pthread_create(&hThd, NULL, read_thd, hQueue)) {
        goto End;
    }

    sleep(2);

    char data[27]; // 0~25: a-z: 26:\0
    memset(data, 0, sizeof(data));
    data[0] = 'a';


    printf("start to push\n");
    for(int i =0; i < COUNT; i++) {
        for(int j =1; j < 26; j++) {
            data[j]= data[j-1] + 1;
        }
        if(0 != xqueue_push(hQueue, data, sizeof(data))) {
            fprintf(stderr, "push fail\n");
            break;
        }
        usleep(2000); //2ms
        if('z' == data[0]) {
            data[0] = 'a';
        } else {
            data[0] += 1;
        }
    }
    pthread_join(hThd, NULL);
End:
    xqueue_destory(hQueue);
    return 0;
}
