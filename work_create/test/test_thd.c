#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "thd_cntl.h"
#include "xqueue.h"
#include "mcu_recv_thd.h"


int main(int argc, char* argv[])
{
    int ret = 0;
    void *queue_mcu2cpu;//, queue_cpu2mcu;
    queue_mcu2cpu = xqueue_new();

    printf("queue new done\n");
    //hThd_t* cpu_send = cpusend_thd_handle_new(queue_mcu2cpu);
    hThd_t* mcu_recv = (hThd_t*)mcurecv_thd_handle_new(queue_mcu2cpu);
    if(NULL == mcu_recv) {
        printf("mcurecv_thd_handle_new fail\n");
        return -1;
    }

    printf("%s, %d: mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, mcu_recv->state);

    //thd_create(cpu_send);
    //thd_start(cpu_send);

    ret = thd_create(mcu_recv);
    printf("%s, %d: thd_create ret: %d, mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_start(mcu_recv);
    printf("%s, %d: thd_start ret: %d, mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_stop(mcu_recv);
    printf("%s, %d: thd_stop ret: %d,  mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_start(mcu_recv);
    printf("%s, %d: thd_start ret: %d, mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_stop(mcu_recv);
    printf("%s, %d: thd_stop ret: %d,  mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_start(mcu_recv);
    printf("%s, %d: thd_start ret: %d, mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_stop(mcu_recv);
    printf("%s, %d: thd_stop ret: %d,  mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_start(mcu_recv);
    printf("%s, %d: thd_start ret: %d, mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    //ret = thd_stop(mcu_recv);
    //printf("%s, %d: thd_stop ret: %d,  mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_destory(mcu_recv);
    printf("%s, %d: thd_destory ret: %d, mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);

    ret = thd_join(mcu_recv);
    printf("%s, %d: thd_join ret: %d, mcu_recv->state: 0x%.2x\n", __FUNCTION__, __LINE__, ret, mcu_recv->state);
    //cpusend_thd_handle_release(mcu_recv);
    mcurecv_thd_handle_release(mcu_recv);

    xqueue_destory(queue_mcu2cpu);
    return 0;
}
