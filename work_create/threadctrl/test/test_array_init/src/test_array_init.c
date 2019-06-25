#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <pthread.h>
#include "sigterm_handler.h"
#include "thd_ctrl.h"


#define gettid() syscall(__NR_gettid)


void input_command()
{
    char buf[128];
    memset(buf, 0, sizeof(128));

    int i = 0;
    printf("INPUT COMMAND or CTRL-D:\n");
    while (NULL != fgets(buf, sizeof(buf), stdin))
    {
        int len = strlen(buf);
        if (len > 0)
        {
            buf[len - 1] = '\0'; //  '\n' to '\0'
        }
        printf("GET COMMAND: %s\n", buf);
        (void)system(buf);
        printf("INPUT COMMAND or CTRL-D:\n");

        memset(buf, 0, sizeof(128));
    }
}

typedef struct can_sig_process
{
    const int sig_id;
    char* target_description;
    char* calc_mode_description;
    char* comment;
    int (*func)(double sig_val, void* data_access_mgr);
} can_sig_proc_t;


int main(int argc, char* argv[])
{

    can_sig_proc_t table[10] =
    {
        [8] = {2, "2-target_description", "{<1.0, 0x01> <2.0, 0x02>}", "", NULL},
        [2] = {2, "8-target_description", "{<8.0, 0x08> <9.0, 0x09>}", "", NULL}
    };

    int i = 0;
    for (i = 0; i < sizeof(table) / sizeof(can_sig_proc_t); i++)
    {
        printf("[%d]: %d, %s, %s, %p, %p\n", i,
               table[i].sig_id,
               table[i].target_description,
               table[i].calc_mode_description,
               table[i].comment,
               table[i].func);

    }
    return 0;
}
