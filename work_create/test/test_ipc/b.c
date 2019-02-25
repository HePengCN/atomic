#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"


int main(int argc, char* argv[]) {
    int ret;
    int shmid;
    key_t key;//, key1;
    //size_t size;
    //int shmflg;
    void* shmaddr = NULL;
    shared_t* shared;

    key = ftok("..", 0xFF);
    //key1 = ftok("/home/he/WinShar", 0xFF);
    //printf("key(%d) %s key1(%d)\n", key, key==key1 ? "==": "!=", key1);
    
    shmid = shmget(key, sizeof(shared_t), IPC_CREAT);
    printf("shmid: %d. error msg: %s\n", shmid, strerror(errno));

    shmaddr = shmat(shmid, NULL, 0);
    printf("shmaddr: %p. error msg: %s\n", shmaddr, strerror(errno));
    //bzero(shmaddr, sizeof(shared_t));

    shared = (shared_t*)shmaddr;
    printf("shared->len: %d; shared->data: %s\n", shared->len, shared->data);

#if 1
    ret = shmctl(shmid, IPC_RMID,NULL);
    printf("shmctl(IPC_RMID): %d. error msg: %s\n", ret, strerror(errno));

    if(-1 == ret) {
        shmid = shmget(key, 100, IPC_CREAT);
        ret = shmctl(shmid, IPC_RMID,NULL);
        printf("try again, shmctl(IPC_RMID): %d. error msg: %s\n", ret, strerror(errno));
    }
#endif
    
    return 0;
}

