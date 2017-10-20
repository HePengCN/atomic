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
#include <cstdint>
#include <string>
#include <memmory>

template <typename T>
std::shared_ptr<T> make_shared_array(size_t size)
{

    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());

}

std::string Time2string(uint64_t utc_ms);

void split(const std::string& s, const std::string& delim, std::vector<std::string>& ret);

time_t  my_gmktime(register struct tm *t);


