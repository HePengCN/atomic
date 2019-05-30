#ifndef __THREAD_PTIMER_H__
#define __THREAD_PTIMER_H__
#include <stdint.h>

typedef void* htimer_t;

/*************************************************
@brief create a thread timer,which means SIGALRM will send to the thread indicated by thread_id;
@param thread_id: Linux lWP id.
       if thead_id set to 0, it means create a timer for the caller thread.
@return handle of timer. NULL will be return if fail.
*************************************************/
htimer_t ptimer_create(pid_t thread_id);


/*************************************************
@brief
@param start_offset_ms:  
           If  specifies  then arms (starts) the timer, setting it to initially expire at the given time. (If
       the timer was already armed, then the previous settings are overwritten.)
       If specifies a zero value then the  timer  is disarmed.

       interval_ms: 
           Specifies the period of the timer.
           If specifies a zero value, then the timer expires just once, at the time specified by start_offset_ms.
@return 0 if ok, others if fail
*************************************************/
int ptimer_set(htimer_t htimer, uint32_t start_offset_ms, uint32_t interval_ms);


/*************************************************
@brief
@param
@return 0 if ok, others if fail
*************************************************/
int ptimer_wait(htimer_t htimer);


/*************************************************
@brief
@param
@return
*************************************************/
void ptimer_destroy(htimer_t htimer);


/*************************************************
@brief calling this function in your main thread to mask SIGALRM in your process.
@param
@return 0 if ok, others if fail
*************************************************/
inline int mask_sigalrm();

#endif //__THREAD_PTIMER_H__
