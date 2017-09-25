/**
*******************************************************************************
*
*
******************************************************************************
* @file
* @brief
*******************************************************************************
**/
#pragma once
#include "utils.hpp"
#include <sys/time.h>

std::string Time2string(uint64_t utc_ms)
{
    std::string str;
    char buf[256];
    memset(buf, 0, sizeof(buf));

    int sub_msec = utc_ms%1000;
    time_t sub_sec = utc_ms/1000;

    struct tm bkTime;
    gmtime_r(&sub_sec, &bkTime);

    snprintf(buf, sizeof(buf), "%4d%.2d%.2d%.2d%.2d%.2d.%03d", (bkTime.tm_year+1900), (bkTime.tm_mon+1), bkTime.tm_mday, bkTime.tm_hour, bkTime.tm_min, bkTime.tm_sec, sub_msec);
    str.assign(buf);
    return str;
}


