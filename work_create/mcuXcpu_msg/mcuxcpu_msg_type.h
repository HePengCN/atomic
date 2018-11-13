#pragma once
#include <stdint.h>

#define MCUXCPU_MSG_MAGIC_NO (0x584D5347) /**<ascii for XMSG>**/

typedef enum mcuxcpu_msg_type {
    XMSG_TYPE_EMPTY = MCUXCPU_MSG_MAGIC_NO,
    XMSG_TYPE_UNKNOWN,
    XMSG_TYPE_ERROR
} mcuxcpu_msg_type_e;

mcuxcpu_msg_type_e msg_type(const void* msg, uint32_t len);

