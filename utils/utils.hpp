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
#include <math.h>

#define MAX(a,b) \
		({\
			typeof(a) _a = a;\
			typeof(b) _b = b;\
			(void)(&_a == &_b);\
			((_a)>(_b)?(_a):(_b));\
		})

#define MIN(a,b) \
		({\
			typeof(a) _a = a;\
			typeof(b) _b = b;\
			(void)(&_a == &_b);\
			((_a)<(_b)?(_a):(_b));\
		})

/*测试
注: void语句的作用，当代码中是两个不同类型比较时，编译会给出警告。但测试的结果是符合代码的。
int main(int argc, char* argv[]) {
	int a = -1;
	int b = 1;
	int c = MAX((unsigned int)a, b);
	int d = MIN((unsigned int)a, b);
	printf("MAX: c = %d. MIN: d = %d\n", c, d);
	return 0;
}
*/


/*
*Notice When Using: must remember to check if(NULL == shard_ptr.get());
*memory alloc fail should be considered.
*/
template <typename T>
std::shared_ptr<T> make_shared_array(size_t size)
{

    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());

}

/*
*Notice When Using: must be sure the order list should be in stric ascending order or descending order.
*1, the Type T can be any original numeric types.
*2, the list can be in ascending order or descending order.
*3, the list must have no duplicated data.
*4, Return value: the index of list;
*/
template <typename T>
int OrderedListSearchTheCloser(std::vector<T> &pstArray, T ullTime)
{
    int iLength = pstArray.size();
    int middle = 0;
    int low = 0, high = iLength - 1;
    if(iLength < 1) {
        printf("%s:%d: error: data array length < 1; length: %d \n", __func__, __LINE__, iLength);
        return -1;
    }

    if(iLength == 1) {
        return iLength - 1;
    }

    if(pstArray[0] < pstArray[iLength - 1]) {
        if(ullTime <= pstArray[0]) {
            return 0;
        } else if(ullTime >= pstArray[iLength - 1]) {
            return iLength - 1;
        }
    } else if(pstArray[0] > pstArray[iLength - 1]) {
        if(ullTime >= pstArray[0]) {
            return 0;
        } else if(ullTime <= pstArray[iLength - 1]) {
            return iLength - 1;
        }
    }

    while(low <= high) {
        middle = (low + high)/2;
        if(abs(pstArray[middle+1] - ullTime) > abs(pstArray[middle] - ullTime)) {
            high = middle - 1;
        } else {
            low = middle + 1;
        }
    }
    return (abs(pstArray[middle+1] - ullTime) > abs(pstArray[middle] - ullTime)) ? middle : (middle+1);
}

/*
*Notice When Using: acording your actual demand, you can change the code that the print format.
*/
std::string Time2string(uint64_t utc_ms);

void split(const std::string& s, const std::string& delim, std::vector<std::string>& ret);

time_t  my_gmktime(register struct tm *t);


/*
*Notice When Using: the return is millisecond;
*acording your actual demand, you can change the code that the string format.
*/
uint64_t gmktimeFrStr(std::string timeStr);


/*
*Notice When Using: only for loading small bin file.
*/
std::string loadBinFile(const char* filePath);


/*
* This is $GPGGA gps Latitude format convert to degree.
*/
double xConvertLat(const std::string &lat);

/*
* This is $GPGGA gps Longitude format convert to degree.
*/
double xConvertLng(const std::string &lng);

/*
*return UTC milliseconds;
*/
uint64_t getCurrentTime();

/*
The input filename may be absolute file path or not
*/
std::string getFileName(std::string filePath, bool withSuffix = true);

/*
*Notice When Using: please be sure the path format is right.
*1, The input filename may be absolute file path or not
*/
std::string getFileDirName(std::string filePath, bool withPrefix = true);


