#ifndef __MSGC_TX_H__
#define __MSGC_TX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "msgc.h"
#include "thd_ctrl.h"
#include "xqueue.h"


hthd_t msgc_tx_hdl_new(const char thdname[16], hqueue_t hqueue);

void msgc_tx_hdl_release(hthd_t hthd);


#ifdef __cplusplus
}
#endif   /* end of __cplusplus */

#endif //__MSGC_TX_H__