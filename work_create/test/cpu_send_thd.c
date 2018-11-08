#include "thd_cntl_impl.h"
#include "cpu_send_thd.h"
#include "xqueue.h"

static void* cpusend_thd(hThd_pthd_cpu_send_t* hThd);
{    
    hThd_pthd_t *hThd_cntl = hThd;
    int ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if(0 != ret) {

    }

    return hThd;
}

hThd_pthd_cpu_send_t* cpusend_thd_handle_new(void* hQueue)
{

}
void cpusend_thd_handle_release(hThd_pthd_cpu_send_t* hThd)
{

}

