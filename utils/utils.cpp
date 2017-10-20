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

void split(const std::string& s, const std::string& delim, std::vector<std::string>& ret)
{
    size_t last = 0;
    size_t index=s.find_first_of(delim,last);
    while (index!=std::string::npos) {
        ret.push_back(s.substr(last,index-last));
        last=index+ delim.length();
        index=s.find_first_of(delim,last);
    }
    if(last < s.length()) {
        ret.push_back(s.substr(last));
    }
}

time_t  my_gmktime(register struct tm *t)
/* struct tm to seconds since Unix epoch */
{
    register   long  year;
    register   time_t  result;
#define  MONTHSPERYEAR   12       /* months per calendar year */
    static   const   int  cumdays[MONTHSPERYEAR] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    /*@ +matchanyintegral @*/
    year = 1900 + t->tm_year + t->tm_mon / MONTHSPERYEAR;
    result = (year - 1970) * 365 + cumdays[t->tm_mon % MONTHSPERYEAR];
    result += (year -  1968) / 4;
    result -= (year -  1900) / 100;
    result += (year -  1600) / 400;
    if  ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0) && (t->tm_mon % MONTHSPERYEAR) < 2) {
        result-- ;
    }
    result += t->tm_mday -  1;
    result *= 24;
    result += t->tm_hour;
    result *= 60;
    result += t->tm_min;
    result *= 60;
    result += t->tm_sec;
    if  (t->tm_isdst == 1)
        result -= 3600;
    /*@ - matchanyintegral @*/
    return  (result);
}

