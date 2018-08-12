/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#include "input_joy.h"
#include "input.h"
#include "timer.h"
#include "debug.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_NOTIFY
    #include <sys/inotify.h>
#endif

#define PATH_MAX 255

static int  init = 0;
static int  devCnt = 0;
static char devNames[INPUT_JOY_MAX][PATH_MAX];

#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_POLL
    static int             taskCycle = 2000; // ms
    static struct timespec lastCycle;
#elif FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_NOTIFY
    static int notify_fd = -1;
#endif

#define NEG_2ND_THRESHOLD    -24576
#define NEG_MID_VALLUE       -16384
#define NEG_1ST_THRESHOLD     -8192
#define POS_1ST_THRESHOLD      8192
#define POS_MID_VALLUE        16384
#define POS_2ND_THRESHOLD     24576

#define AX_INIT_STATE 127

static const int16_t THRESHOLDS[3][2] = {
    {INT16_MIN,         NEG_1ST_THRESHOLD},
    {NEG_2ND_THRESHOLD, POS_2ND_THRESHOLD},
    {POS_1ST_THRESHOLD, INT16_MAX}
};

typedef struct joy_data
{
    int                 fd;
    uint8_t             cntAxes;
    uint8_t             cntButtons;
    char                name[80];
    struct js_corr      axCorr[ABS_CNT];
    uint8_t             axMap[ABS_CNT];
    int16_t             axCurVal[ABS_CNT];
    int8_t              axCurState[ABS_CNT]; // -1, 0, 1, init AX_INIT_STATE
    uint16_t            btnMap[KEY_MAX - BTN_MISC + 1];
} joy_data_t;

static joy_data_t       joys[INPUT_JOY_MAX];


static int joy_left[]     = {BTN_DPAD_LEFT,  0};
static int joy_right[]    = {BTN_DPAD_RIGHT, 0};
static int joy_up[]       = {BTN_DPAD_UP,    0};
static int joy_down[]     = {BTN_DPAD_DOWN,  0};
static int joy_select[]   = {BTN_JOYSTICK, BTN_THUMB, BTN_THUMB2, BTN_TOP, BTN_TOP2, BTN_PINKIE,
                            BTN_BASE, BTN_BASE2, BTN_BASE3, BTN_BASE4, BTN_BASE5, BTN_BASE6,
                            BTN_DEAD, BTN_A, BTN_B, BTN_C, BTN_X, BTN_Y, BTN_Z,
                            BTN_TL, BTN_TR, BTN_TL2, BTN_TR2, BTN_SELECT, BTN_START, BTN_MODE,
                            BTN_THUMBL, BTN_THUMBR, 0};

static event_map joy_event_map = {NULL, 
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  joy_left,
                                  joy_right,
                                  joy_up,
                                  joy_down,
                                  joy_select,
                                  NULL};

static void joy_close(int devNr);
static input_event getJoyEvent(int devNr);
#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_NOTIFY
    static void handleNotifyEvent();
#endif
static input_event axes2Event(uint8_t axisId, int8_t state);


int joy_cfgAddDev(const char *devName)
{
    if (!init && (devCnt < INPUT_JOY_MAX))
    {
        strncpy(devNames[devCnt], devName, PATH_MAX);
        devNames[devCnt][PATH_MAX-0] = 0; // be shure always be null-terminated
        ++devCnt;
        return 0;
    }
    else
    {
        return -1;
    }
}


int joy_init()
{
    if (!init)
    {
        int devNr;

        if (devCnt == 0)
        {
            joy_cfgAddDev("/dev/input/js0");
            joy_cfgAddDev("/dev/input/js1");
            joy_cfgAddDev("/dev/input/js2");
            joy_cfgAddDev("/dev/input/js3");
            joy_cfgAddDev("/dev/input/js4");
            joy_cfgAddDev("/dev/input/js5");
            joy_cfgAddDev("/dev/input/js6");
            joy_cfgAddDev("/dev/input/js7");
            joy_cfgAddDev("/dev/input/js8");
            joy_cfgAddDev("/dev/input/js9");
        }

        memset(joys, 0, sizeof(joys));
        for (devNr = 0; devNr < devCnt; ++devNr)
        {
            joys[devNr].fd = -1;
            memset(joys[devNr].axCurState, AX_INIT_STATE, sizeof(joys[devNr].axCurState));
        }
#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_NOTIFY
        notify_fd = inotify_init();

        if (notify_fd >= 0)
        {
            int wd;

            wd = inotify_add_watch(notify_fd, "/dev/input", IN_ATTRIB | IN_CREATE);
            if (wd < 0)
            {
                debugOut(debug_level0, "Err inotify_add_watch: %d\n", errno);
            }
            // TODO also watch /dev/input/by-id and /dev/input/by-path
        }
        else
        {
            debugOut(debug_level0, "Err inotify_init: %d\n", errno);
        }
#endif
        init = 1;

        joy_task();

        return 0;
    }
    else
    {
        return -1;
    }
}


void joy_task()
{
    int devNr;
    for (devNr = 0; init && (devNr < devCnt); ++devNr)
    {
        if (joys[devNr].fd < 0)
        {
            joys[devNr].fd = open(devNames[devNr], O_RDONLY | O_ASYNC);
            if (joys[devNr].fd >= 0)
            {
                int i, x;

                debugOut(debug_level2, "Joystick found [%d](%s) fd:%d:\n",
                    devNr, devNames[devNr], joys[devNr].fd);

                ioctl(joys[devNr].fd, JSIOCGAXES, &joys[devNr].cntAxes);
                ioctl(joys[devNr].fd, JSIOCGBUTTONS, &joys[devNr].cntButtons);
                ioctl(joys[devNr].fd, JSIOCGNAME(sizeof(joys[devNr].name)), &joys[devNr].name);
                ioctl(joys[devNr].fd, JSIOCGCORR, joys[devNr].axCorr);
                ioctl(joys[devNr].fd, JSIOCGAXMAP, joys[devNr].axMap);
                ioctl(joys[devNr].fd, JSIOCGBTNMAP, joys[devNr].btnMap);

                debugOut(debug_level2, "cntAxes:    %d\n", joys[devNr].cntAxes);
                debugOut(debug_level2, "cntButtons: %d\n", joys[devNr].cntButtons);
                debugOut(debug_level2, "name:       %s\n", joys[devNr].name);
                debugOut(debug_level2, "ax: ");
                for (i = 0; i < joys[devNr].cntAxes; ++i)
                {
                    debugOut(debug_level2, "%d:%02x, ", i, joys[devNr].axMap[i]);
                }
                debugOut(debug_level2, "\n");
                debugOut(debug_level2, "axCorr: ");
                for (i = 0; i < joys[devNr].cntAxes; ++i)
                {
                    debugOut(debug_level2, "%d:%02x %d [",
                        i, joys[devNr].axCorr[i].type, joys[devNr].axCorr[i].prec);
                    for (x = 0; x < 8; ++x)
                    {
                        debugOut(debug_level2, "%d, ", joys[devNr].axCorr[i].coef[x]);
                    }
                    debugOut(debug_level2, "]\n");
                }
                debugOut(debug_level2, "\n");
                debugOut(debug_level2, "btn: ");
                for (i = 0; i < joys[devNr].cntButtons; ++i)
                {
                    debugOut(debug_level2, "%d:%03x, ", i, joys[devNr].btnMap[i]);
                }
                debugOut(debug_level2, "\n");
            }
        }
    }

#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_POLL
    getCurClock(&lastCycle);
#endif
}


int joy_getTaskTimeout()
{
#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_POLL
    if (init)
    {
        int to = getTimeout(&lastCycle, taskCycle);
        if (to < 0)
        {
            joy_task();
            return taskCycle;
        }
        return to;
    }
    else
#endif
    {
        return -1;
    }
}


void joy_stop()
{
    if (init)
    {
        int devNr;
        for (devNr = 0; devNr < devCnt; ++devNr)
        {
            joy_close(devNr);
        }
        init = 0;
    }
}


static void joy_close(int devNr)
{
    assert(devNr < INPUT_JOY_MAX);

    if (init)
    {
        if (joys[devNr].fd >= 0)
        {
            close(joys[devNr].fd);
            joys[devNr].fd = -1;
            memset(joys[devNr].axCurState, AX_INIT_STATE, sizeof(joys[devNr].axCurState));
        }
    }
}


int joy_cntOpen()
{
    return devCnt;
}


int joy_getFd(int devNr)
{
    if (init)
    {
        if (devNr < INPUT_JOY_MAX)
        {
            return joys[devNr].fd;
        }

#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_NOTIFY
        if (devNr == INPUT_JOY_MAX)
        {
            return notify_fd;
        }
#endif

        return -1;
    }
    else
    {
        return -1;
    }
}


input_event joy_getEvent(int devNr)
{
    if (init)
    {
        if (devNr < INPUT_JOY_MAX)
        {
            if (joys[devNr].fd >= 0)
            {
                return getJoyEvent(devNr);
            }
        }

#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_NOTIFY
        if (devNr == INPUT_JOY_MAX)
        {
            handleNotifyEvent();
        }
#endif
    }

    return input_none;
}


static input_event getJoyEvent(int devNr)
{
    struct js_event event;

    ssize_t retval;

    retval = read(joys[devNr].fd, &event, sizeof(event));

    if (retval < 0)
    {
        debugOut(debug_level0, "Error reading joy: %d\n", errno);
        joy_close(devNr);
    }
    else if (retval == 0)
    {
        debugOut(debug_level0, "NoData from joy\n");
        joy_close(devNr);
    }
    else if (retval == sizeof(event))
    {
        if (event.type == JS_EVENT_BUTTON)
        {
            debugOut(debug_level3, "Joy %d Btn Time:%x #:%d[%02x] data:%d\n",
                devNr, event.time, event.number, joys[devNr].btnMap[event.number], event.value);
            if (event.value)
            {
                return key2event(joy_event_map, joys[devNr].btnMap[event.number]);
            }
        }
        else if (event.type == JS_EVENT_AXIS)
        {
            debugOut(debug_level3, "Joy %d Ax Time:%x #:%d[%02x] data:%d\n",
                devNr, event.time, event.number, joys[devNr].axMap[event.number], event.value);

            if (   (joys[devNr].axCurState[event.number] >= -1)
                && (joys[devNr].axCurState[event.number] <= 1))
            {
                int i = joys[devNr].axCurState[event.number] + 1;
                int8_t newState = joys[devNr].axCurState[event.number];

                if (event.value < THRESHOLDS[i][0])
                {
                    --newState;
                }
                else if (event.value > THRESHOLDS[i][1])
                {
                    ++newState;
                }
                if (newState != joys[devNr].axCurState[event.number])
                {
                    joys[devNr].axCurState[event.number] = newState;
                    debugOut(debug_level3, "NewState %d\n", newState);

                    return axes2Event(joys[devNr].axMap[event.number], newState);
                }
            }
        }
        else if ((JS_EVENT_INIT & event.type) == JS_EVENT_INIT)
        {
            if ((event.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS)
            {
                if (joys[devNr].axCurState[event.number] == AX_INIT_STATE)
                {
                    joys[devNr].axCurVal[event.number] = event.value;
                    if (event.value < NEG_MID_VALLUE)
                    {
                        joys[devNr].axCurState[event.number] = -1;
                    }
                    else if (event.value > POS_MID_VALLUE)
                    {
                        joys[devNr].axCurState[event.number] = 1;
                    }
                    else
                    {
                        joys[devNr].axCurState[event.number] = 0;
                    }
                }
            }
        }
    }
    else
    {
        debugOut(debug_level0, "Not enough data from joy\n");
        joy_close(devNr);
    }

    return input_none;
}


#if FRABENU_JOYMONITORMODE == FRABENU_JOYMONITORMODE_NOTIFY

static void handleNotifyEvent()
{
    char buf[4096];

    read(notify_fd, buf, sizeof(buf)); // read all data, to be able to poll again for notify_fd
                                       // but we don't need the data it's just a trigger
    joy_task();
}

#endif


static input_event axes2Event(uint8_t axisId, int8_t state)
{
    input_event event = input_none;

    switch (axisId)
    {
        case ABS_X:
        case ABS_RX:
        case ABS_HAT0X:
        case ABS_HAT1X:
        case ABS_HAT2X:
        case ABS_HAT3X:
            if      (state == -1) { event = input_left;  }
            else if (state ==  1) { event = input_right; }
            break;
        case ABS_Y:
        case ABS_RY:
        case ABS_HAT0Y:
        case ABS_HAT1Y:
        case ABS_HAT2Y:
        case ABS_HAT3Y:
            if      (state == -1) { event = input_up;   }
            else if (state ==  1) { event = input_down; }
            break;
    }

    return event;
}
