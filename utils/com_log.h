#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <libgen.h>
#include <inttypes.h>
#define COMPILE_WITH_SYSTEM

#ifdef COMPILE_WITH_SYSTEM
#include "log.h"
#define COM_LOG_DEBUG(TAG, fmt, args...) LOG_DEBUG("["TAG"]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_INFO(TAG, fmt, args...) LOG_INFO("["TAG"]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_WARN(TAG, fmt, args...) LOG_WARN("["TAG"]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#define COM_LOG_ERROR(TAG, fmt, args...) LOG_ERROR("["TAG"]%s Line %d: "fmt, basename(__FILE__), __LINE__, ##args)
#else
#include <stdio.h>
#define COM_LOG_DEBUG(TAG, fmt, ...) printf("[DEBUG], "TAG", %s[%d]: %s: "fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define COM_LOG_INFO(TAG, fmt, ...) printf("[INFO], "TAG", %s[%d]: %s: "fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define COM_LOG_WARN(TAG, fmt, ...) fprintf(stderr, "[WARING], "TAG", %s[%d] %s: "fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define COM_LOG_ERROR(TAG, fmt, ...) fprintf(stderr, "[ERROR], "TAG", %s[%d] %s: "fmt, basename(__FILE__), __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif
