#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "timer.h"

void seconds_sleep(unsigned seconds)
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    int err;
    do
    {
        err = select(0, NULL, NULL, NULL, &tv);
    }
    while (err < 0 && errno == EINTR);
}

void milliseconds_sleep(unsigned long mSec)
{
    struct timeval tv;
    tv.tv_sec = mSec / 1000;
    tv.tv_usec = (mSec % 1000) * 1000;
    int err;
    do
    {
        err = select(0, NULL, NULL, NULL, &tv);
    }
    while (err < 0 && errno == EINTR);
}

void microseconds_sleep(unsigned long uSec)
{
    struct timeval tv;
    tv.tv_sec = uSec / 1000000;
    tv.tv_usec = uSec % 1000000;
    int err;
    do
    {
        err = select(0, NULL, NULL, NULL, &tv);
    }
    while (err < 0 && errno == EINTR);
}

uint64_t get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (uint64_t)1000 + tv.tv_usec / (uint64_t)1000;
}

