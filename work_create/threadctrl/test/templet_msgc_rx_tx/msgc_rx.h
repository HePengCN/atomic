#ifndef __MSGC_RX_H__
#define __MSGC_RX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xqueue.h"
#include "thd_ctrl.h"


hthd_t msgc_rx_hdl_new(const char thdname[16], hqueue_t hqueue);

void msgc_rx_hdl_release(hthd_t hthd_base);


#ifdef __cplusplus
}
#endif   /* end of __cplusplus */

#endif //__MSGC_RX_H__
