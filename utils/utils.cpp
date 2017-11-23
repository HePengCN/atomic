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

/*
*Notice When Using: acording your actual demand, you can change the print format.
*/
std::string Time2string(uint64_t utc_ms)
{
#define PRINTFORMAT "%4d%.2d%.2d%.2d%.2d%.2d.%03d"

    std::string str;
    char buf[256];
    memset(buf, 0, sizeof(buf));

    int sub_msec = utc_ms%1000;
    time_t sub_sec = utc_ms/1000;

    struct tm bkTime;
    gmtime_r(&sub_sec, &bkTime);

    snprintf(buf, sizeof(buf), PRINTFORMAT, (bkTime.tm_year+1900), (bkTime.tm_mon+1), bkTime.tm_mday, bkTime.tm_hour, bkTime.tm_min, bkTime.tm_sec, sub_msec);
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

/*
*Notice When Using: the return is millisecond;
*Acording your actual demand, you can change the code that the string format, Only keep the indentitors(YYYY, MM, mm etc.) are unique.
*You must be sure the timeStr are in right format, otherwise, the result is undfined.
*/
uint64_t gmktimeFrStr(std::string timeStr)
{
    const std::string STRFORMAT = "YYYY-MM-DD_T_hh-mm-ss.xxx";

    int ms;
    time_t sec;
    struct tm brk_tm;
    brk_tm.tm_year = atoi(timeStr.substr(STRFORMAT.find('Y'), 4).c_str());
    brk_tm.tm_mon  = atoi(timeStr.substr(STRFORMAT.find('M'), 2).c_str());
    brk_tm.tm_mday = atoi(timeStr.substr(STRFORMAT.find('D'), 2).c_str());
    brk_tm.tm_hour = atoi(timeStr.substr(STRFORMAT.find('h'), 2).c_str());
    brk_tm.tm_min  = atoi(timeStr.substr(STRFORMAT.find('m'), 2).c_str());
    brk_tm.tm_sec  = atoi(timeStr.substr(STRFORMAT.find('s'), 2).c_str());
    ms             = atoi(timeStr.substr(STRFORMAT.find('x'), 3).c_str());

    brk_tm.tm_year -= 1900;
    brk_tm.tm_mon  -= 1;

    sec = my_gmktime(&brk_tm);
    return sec*(uint64_t)1000 + ms;
}


/*
*Notice When Using: only for loading small bin file.
*/
std::string loadBinFile(const char* filePath)
{
    std::string str;
    struct stat statBuf;
    if(0 != stat(filePath, &statBuf)) {
        printf("%s:%d: Error: stat file %s fail\n", __FUNCTION__, __LINE__, filePath);
        return str;
    }
    str.resize(statBuf.st_size);

    FILE *fd = fopen(filePath, "r");
    if(!fd) {
        printf("%s:%d: Error: open file %s fail\n", __FUNCTION__, __LINE__, filePath);
        return str;
    }

    fread(const_cast<char*>(str.data()), 1, statBuf.st_size, fd);
    fclose(fd);
    return str;
}

/*
* This is $GPGGA gps Latitude format convert to degree.
*/
double xConvertLat(const std::string &lat)
{
    /*for example: 4229.42689*/
    int degree = atoi(lat.substr(0, 2).c_str());
    double min = atof(lat.substr(2).c_str());
    return degree + min/60.0;
}

/*
* This is $GPGGA gps Longitude format convert to degree.
*/
double xConvertLng(const std::string &lng)
{
    /*for example: 08308.18995*/
    int degree = atoi(lng.substr(0, 3).c_str());
    double min = atof(lng.substr(3).c_str());
    return degree + min/60.0;
}

bool getTotalBytesOfRecFiles(std::string& folder_path, uint64_t &totalBytes)
{
    totalBytes = 0;
    DIR *dir;
    struct dirent *ptr;

    if (NULL == (dir=opendir(folder_path.c_str()))) {
        e(CONTROLTAG,"Open folder %s fail: %s", folder_path.c_str(), strerror(errno));
        return false;
    }

    while ((ptr=readdir(dir)) != NULL) {
        if(0 == strcmp(ptr->d_name,".") || 0 == strcmp(ptr->d_name,".."))
            continue;
        else if(DT_REG == ptr->d_type) {
            size_t len = strlen(ptr->d_name);
            if(len < 4) {
                continue;
            }
            if(0 == strcmp(&((ptr->d_name)[len-4]),".rtv")) {
                std::string file(folder_path);
                file.append("/");
                file.append(ptr->d_name);
                struct stat statbuf;
                stat(file.c_str(), &statbuf);
                totalBytes += statbuf.st_size;
            }
        } else if(DT_LNK == ptr->d_type) {
        } else if(DT_DIR == ptr->d_type) {
        }

    }
    closedir(dir);
    return true;
}

uint64_t getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * (uint64_t)1000 + tv.tv_usec / (uint64_t)1000;
}

/*
*Notice When Using: please be sure the path format is right.
*1, The input filename may be absolute file path or not
*/
std::string getFileName(std::string filePath, bool withSuffix)
{
    size_t pos = filePath.rfind('/');
    if(std::string::npos == pos) {
        return filePath;
    }

    if(withSuffix) {
        return filePath.substr(pos+1);
    } else {
        return filePath.substr(pos+1, (filePath.rfine('.')-1) - pos);
    }
}

/*
*Notice When Using: please be sure the path format is right.
*1, The input filename may be absolute file path or not
*/
std::string getFileDirName(std::string filePath, bool withPrefix)
{
    size_t pos = filePath.rfind('/');
    if(std::string::npos == pos) {
        return ".";
    }

    std::string res = filePath.substr(0, pos);

    if(withPrefix) {
        return res;
    } else {
        pos = res.rfind('/');
        if(std::string::npos == pos) {
            return res;
        }
        return res.substr(pos+1);
    }
}


