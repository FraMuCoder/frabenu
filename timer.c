/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#include "timer.h"
#include <limits.h>

void timespecDiff(const struct timespec *first, const struct timespec *second, struct timespec *diff)
{
    diff->tv_sec = second->tv_sec - first->tv_sec;
    if (second->tv_nsec >= first->tv_nsec)
    {
        diff->tv_nsec = second->tv_nsec - first->tv_nsec;
    }
    else
    {
        diff->tv_nsec = 1000000000L + second->tv_nsec - first->tv_nsec;
        --diff->tv_sec;
    }
}


void ms2timespec(int ms, struct timespec *ts)
{
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}


int timespec2ms(const struct timespec *ts)
{
    int ms = ts->tv_nsec / 1000000;
    if (ts->tv_sec >= ((INT_MAX / 1000) - ms))
    {
        return INT_MAX;
    }
    ms += ts->tv_sec * 1000;
    return ms;
}


int cmpTimespec(const struct timespec *first, const struct timespec *second)
{
    if (first->tv_sec == second->tv_sec)
    {
        if (first->tv_nsec == second->tv_nsec)
        {
            return 0;
        }
        else if (first->tv_nsec < second->tv_nsec)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else if (first->tv_sec < second->tv_sec)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}


int getTimeout(const struct timespec *start, int timeout)
{
    if (timeout > 0)
    {
        struct timespec cur;
        struct timespec diff;
        struct timespec to_ts;
        struct timespec diffToTO;
        int retval;

        retval = getCurClock(&cur);
        if (retval) { return -1; }
        timespecDiff(start, &cur, &diff);
        ms2timespec(timeout, &to_ts);

        retval = cmpTimespec(&diff, &to_ts);
        if (retval < 0) // diff < to_ts
        {
            timespecDiff(&diff, &to_ts, &diffToTO); // diffToTO = to_ts - diff
            return timespec2ms(&diffToTO);
        }
        else
        {
            return -1; // timeout already occurred
        }
    }
    else
    {
        return -1;
    }
}


int getMinTimeout(int curTO, int nextTO)
{
    if (curTO >= 0)
    {
        if (nextTO >= 0)
        {
            if (nextTO < curTO) { return nextTO; }
            else                { return curTO;  }
        }
        else
        {
            return curTO;
        }
    }
    else
    {
        return nextTO;
    }
}
