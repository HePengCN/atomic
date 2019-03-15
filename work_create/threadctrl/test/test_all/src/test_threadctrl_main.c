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
#include "thd_ctrl.h"
#include "xqueue.h"
#include "consumer_thread.h"
#include "producer_thread.h"

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


#define MAX_MSG_LENGTH (128)

int main(int argc, char* argv[])
{

    printf("Test start: pid: %d.\n", getpid());
    int ret = 0;
    hqueue_t hqueue;
    hqueue = xqueue_create(MAX_MSG_LENGTH);


    hthd_t consumer_thread_handler, producer_thread_handler;
    consumer_thread_handler = consumer_thread_handler_new("consumer_123", hqueue);
    producer_thread_handler = producer_thread_handler_new("procuder_123", hqueue);


    //hthd_t consumer_thread_handler2;
    //consumer_thread_handler2 = consumer_thread_handler_new("consumer_456", hqueue);

#if 0
    thd_sched_param_t sched_param = {THD_SCHED_RR, 70};

    thd_sched_param_t out_sched_param = {THD_SCHED_RR, 70};

    thd_set_sched(consumer_thread_handler, &sched_param);

    thd_get_sched(consumer_thread_handler, &out_sched_param);
    printf("%s: policy: %s, priority: %d\n", thd_name(consumer_thread_handler), thd_sched_policy_str(out_sched_param.sched_policy), out_sched_param.sched_priority);
#endif

    //thd_create(consumer_thread_handler);
    thd_create_and_start(consumer_thread_handler);
    //thd_create_and_start(consumer_thread_handler2);

    thd_create_and_start(producer_thread_handler);

    //input_command();
    printf("main thread sleep in\n");
    usleep(10 * 1000);
    printf("main thread sleep out\n");
#if 0
    thd_get_sched(consumer_thread_handler, &out_sched_param);
    printf("%s: policy: %s, priority: %d\n", thd_name(consumer_thread_handler), thd_sched_policy_str(out_sched_param.sched_policy), out_sched_param.sched_priority);

    thd_get_sched(producer_thread_handler, &out_sched_param);
    printf("%s: policy: %s, priority: %d\n", thd_name(producer_thread_handler), thd_sched_policy_str(out_sched_param.sched_policy), out_sched_param.sched_priority);
#endif
    //thd_join(consumer_thread_handler);

    //sleep(1);

    //thd_pause(producer_thread_handler);

#if 0
    printf("producer state: %s\n", thd_state_str(thd_state(producer_thread_handler)));
    printf("consumer state: %s\n", thd_state_str(thd_state(consumer_thread_handler)));

    thd_get_sched(consumer_thread_handler, &out_sched_param);
    printf("%s: policy: %s, priority: %d\n", thd_name(consumer_thread_handler), thd_sched_policy_str(out_sched_param.sched_policy), out_sched_param.sched_priority);

    thd_get_sched(producer_thread_handler, &out_sched_param);
    printf("%s: policy: %s, priority: %d\n", thd_name(producer_thread_handler), thd_sched_policy_str(out_sched_param.sched_policy), out_sched_param.sched_priority);
#endif

    //sleep(3);

    //thd_start(producer_thread_handler);

#if 0
    printf("producer state: %s\n", thd_state_str(thd_state(producer_thread_handler)));
    printf("consumer state: %s\n", thd_state_str(thd_state(consumer_thread_handler)));

    thd_get_sched(consumer_thread_handler, &out_sched_param);
    printf("%s: policy: %s, priority: %d\n", thd_name(consumer_thread_handler), thd_sched_policy_str(out_sched_param.sched_policy), out_sched_param.sched_priority);

    thd_get_sched(producer_thread_handler, &out_sched_param);
    printf("%s: policy: %s, priority: %d\n", thd_name(producer_thread_handler), thd_sched_policy_str(out_sched_param.sched_policy), out_sched_param.sched_priority);
#endif

#if 1
    sleep(3);


    thd_pause(consumer_thread_handler);
    thd_pause(producer_thread_handler);

    printf("producer state: %s\n", thd_state_str(thd_state(producer_thread_handler)));
    printf("consumer state: %s\n", thd_state_str(thd_state(consumer_thread_handler)));

    sleep(3);

    thd_start(consumer_thread_handler);
    thd_start(producer_thread_handler);

    printf("producer state: %s\n", thd_state_str(thd_state(producer_thread_handler)));
    printf("consumer state: %s\n", thd_state_str(thd_state(consumer_thread_handler)));

    sleep(3);

    thd_pause(consumer_thread_handler);
    thd_pause(producer_thread_handler);

    printf("producer state: %s\n", thd_state_str(thd_state(producer_thread_handler)));
    printf("consumer state: %s\n", thd_state_str(thd_state(consumer_thread_handler)));

    sleep(3);

    thd_start(consumer_thread_handler);
    thd_start(producer_thread_handler);

    printf("producer state: %s\n", thd_state_str(thd_state(producer_thread_handler)));
    printf("consumer state: %s\n", thd_state_str(thd_state(consumer_thread_handler)));


    sleep(3);
#endif

#if 0
    thd_pause(consumer_thread_handler);
    thd_pause(producer_thread_handler);

    printf("producer state: %s\n", thd_state_str(thd_state(producer_thread_handler)));
    printf("consumer state: %s\n", thd_state_str(thd_state(consumer_thread_handler)));

    sleep(3);
#endif

    printf("main thread destroy in\n");

    ret = thd_destroy(consumer_thread_handler);

    thd_destroy(producer_thread_handler);
    //thd_destroy(consumer_thread_handler2);


    printf("thd_destoy return %d\n", ret);
    //input_command();
    //sleep(1);


    //printf("producer state: %s\n", thd_state_str(thd_state(producer_thread_handler)));
    //printf("consumer state: %s\n", thd_state_str(thd_state(consumer_thread_handler)));

#if 1
    consumer_thread_handler_release(consumer_thread_handler);
    producer_thread_handler_release(producer_thread_handler);


    //printf("%s, %d\n", __FUNCTION__, __LINE__);
    xqueue_destory(hqueue);
#endif
    printf("Test end: pid: %d\n", getpid());
    input_command();
    //sleep(10);
    return 0;
}
