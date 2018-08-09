/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#ifndef _FRABENU_TIMER_H_
#define _FRABENU_TIMER_H_

#include <time.h>

#define getCurClock(cur) clock_gettime(CLOCK_MONOTONIC, cur)

/**
 * @brief Get remaining timeout in ms.
 * @param start     Start point (use getCurClock(&start))
 * @param timeout   Total timeout in ms.
 * @return          timeout - (now - start)
 */
int getTimeout(const struct timespec *start, int timeout);

/**
 * @brief Get min. timeout value which is >= 0.
 *
 * If both value are less than 0, curTO or nextTO may returned.
 * @param curTO     Timeout value a
 * @param nextTO    Timeout value b
 * @return          The lowes value >= 0
 */
int getMinTimeout(int curTO, int nextTO);

#endif //_FRABENU_TIMER_H_
