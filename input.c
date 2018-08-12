/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#include "debug.h"
#include "input.h"
#include "input_kbd.h"
#include "input_joy.h"
#include "timer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


int input_init(void)
{
    int err = 0;
    err = kbd_init();
    err |= joy_init();
    return err;
}


void input_stop(void)
{
    joy_stop();
    kbd_stop();
}


static int lastFdNr = -1;

input_event input_get(void)
{
    input_event event = input_none;
    struct pollfd fds[INPUT_JOY_MAXFD + INPUT_KBD_MAX];
    const int nfds = sizeof(fds) / sizeof(fds[0]);
    int timeout = -1; // ms
    int retval;
    int i;

    memset(fds, 0, sizeof(fds));

    for (i = 0; i < INPUT_JOY_MAXFD; ++i)
    {
        fds[i].fd = joy_getFd(i);
        fds[i].events = POLLIN;
    }

    for (i = 0; i < INPUT_KBD_MAX; ++i)
    {
        fds[INPUT_JOY_MAXFD+i].fd = kbd_getFd(i);
        fds[INPUT_JOY_MAXFD+i].events = POLLIN;
    }

    timeout = getMinTimeout(timeout, kbd_getTaskTimeout());
    timeout = getMinTimeout(timeout, joy_getTaskTimeout());

    retval = poll(fds, nfds, timeout);

    if (-1 == retval)       // error
    {
        debugOut(debug_level0, "Error\n");
        return input_abort;
    }
    else if (retval)        // have data
    {
        ++lastFdNr;
        for (i = 0; i < nfds; ++i)
        {
            int fdNr = lastFdNr + i;
            if (fdNr >= nfds) { fdNr -= nfds; }
            if (fds[fdNr].revents)
            {
                if (fdNr < INPUT_JOY_MAXFD)
                {
                    event = joy_getEvent(fdNr);
                }
                else
                {
                    event = kbd_getEvent(fdNr - INPUT_JOY_MAXFD);
                }
                lastFdNr = fdNr;
                break; // hande only one event
            }
        }
    }
    else                    // timeout
    {
        // nothing more to do
    }

    return event;
}


input_event key2event(const event_map map, int key)
{
    input_event event = input_none;
    int e, n;

    for (e = 0; e < input_event_cnt; ++e)
    {
        if (map[e] != NULL)
        {
            for(n = 0; map[e][n] != 0; ++n)
            {
                if (map[e][n] == key)
                {
                    event = (input_event)e;
                    break;
                }
            }
            if (event != input_none)
            {
                break;
            }
        }
    }

    return event;
}
