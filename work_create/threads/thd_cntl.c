#include "thd_cntl.h"
#include "thd_cntl_impl.h"

int thd_create(hThd_t *hThd)
{
    return thd_impl_create((hThd_pthd_t*)hThd);
}

int thd_start(hThd_t *hThd)
{
    return thd_impl_start((hThd_pthd_t*)hThd);
}

int thd_stop(hThd_t *hThd)
{
    return thd_impl_stop((hThd_pthd_t*)hThd);
}

int thd_destory(hThd_t *hThd)
{
    return thd_impl_destory((hThd_pthd_t*)hThd);
}

int thd_join(hThd_t *hThd)
{
    return thd_impl_join((hThd_pthd_t*)hThd);
}

thd_state_e thd_state(hThd_t *hThd)
{
    return thd_impl_state((hThd_pthd_t*)hThd);
}

char*  thd_state_str(thd_state_e state)
{
    static char strs[THD_STATE_EXITED-THD_STATE_UNCREAT+1][32] = {"THD_STATE_UNCREAT",
                                                                  "THD_STATE_CREATED",
                                                                  "THD_STATE_INITED",
                                                                  "THD_STATE_RUNNING",
                                                                  "THD_STATE_STOPPED",
                                                                  "THD_STATE_EXITED"
                                                                 };
    return strs[state-THD_STATE_UNCREAT];
}

