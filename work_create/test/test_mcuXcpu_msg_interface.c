#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "mcuXcpu_msg.h"


int main(int argc, char* argv[])
{

    void *msg_empty, *msg_800;
    msg_empty = mcuXcpu_msg_new(0);
    msg_800 = mcuXcpu_msg_new(800+1);

    int len = mcuXcpu_msg_size(msg_800);
    char *pstr = mcuXcpu_msg_data(msg_800);
    const char* pcon_str = mcuXcpu_msg_data_const(msg_800);

    for(int i = 0; i < 80; i ++) {
        strncpy(pstr+i*10, "0123456789", 10);
    }

    printf("content:\n%s\n", pcon_str);

    mcuXcpu_msg_type_e msg_type = mcuXcpu_msg_type(msg_empty);
    printf("empty msg_type: 0x%.2x\n", msg_type);
    mcuXcpu_msg_destory(msg_empty);
    mcuXcpu_msg_destory(msg_800);
    return 0;
}

