#ifndef __XSTACK_LOG_H__
#define __XSTACK_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <libgen.h>
#include <inttypes.h>

#ifdef COMPILE_WITH_SYSTEM
#include "log.h"
#define COM_LOG_DEBUG(fmt, args...) LOG_DEBUG("[XSTACK]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_INFO(fmt, args...) LOG_INFO("[XSTACK]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_WARN(fmt, args...) LOG_WARN("[XSTACK]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_ERROR(fmt, args...) LOG_ERROR("[XSTACK]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#else
#include <stdio.h>
#define COM_LOG_DEBUG(fmt, ...) printf("[DEBUG], %s[%d]: %s: "fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define COM_LOG_INFO(fmt, ...) printf("[INFO], %s[%d]: %s: "fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define COM_LOG_WARN(fmt, ...) fprintf(stderr, "[WARING], %s[%d] %s: "fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define COM_LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR], %s[%d] %s: "fmt, basename(__FILE__), __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif //__XSTACK_LOG_H__