#ifndef __THD_CTRL_LOG_H_
#define __THD_CTRL_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <libgen.h>
#include <inttypes.h>

#ifdef COMPILE_WITH_SYSTEM
#include "log.h"
#define COM_LOG_DEBUG(fmt, args...) LOG_DEBUG("[THD_CTRL]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_INFO(fmt, args...) LOG_INFO("[THD_CTRL]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_WARN(fmt, args...) LOG_WARN("[THD_CTRL]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_ERROR(fmt, args...) LOG_ERROR("[THD_CTRL]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
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

#endif //__THD_CTRL_LOG_H_