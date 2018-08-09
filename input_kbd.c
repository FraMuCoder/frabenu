/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#include "input_kbd.h"
#include "debug.h"
#include "timer.h"
#include <linux/input.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

typedef enum CSISeqState
{
    CSI_none,       // no CSI sequence ist stated
    CSI_ESCread,    // just ESC readed
    CSI_waitForEnd, // ESC+[ readed, wait for end of seq
} CSISeqState;

static int  init = 0;
static CSISeqState curCSISeqState = CSI_none;
static int kbdPipe[2] = { -1, -1 };  // 0-r, 1-w
static int escTimeout = 1000; // ms
static struct timespec startCSI;

#define CSI_1  0x1B     /* ESC */
#define CSI_2  0x5B     /* [   */
#define CSI_F_FIRST 0x40
#define CSI_F_LAST  0x7E

typedef struct {
    uint8_t     stdIn;      // normal stdIn data or CSI final byte
    uint16_t    keyCode;    // key code
} CSI_F_2_Key;

static CSI_F_2_Key mapNormal_2_KeyCode[] = {
    {0x1B, KEY_ESC},
    {'0',  KEY_0},
    {'1',  KEY_1},
    {'2',  KEY_2},
    {'3',  KEY_3},
    {'4',  KEY_4},
    {'5',  KEY_5},
    {'6',  KEY_6},
    {'7',  KEY_7},
    {'8',  KEY_8},
    {'9',  KEY_9},
    {' ',  KEY_SPACE},
    {'q',  KEY_Q},
    {'Q',  KEY_Q},
    {0x0A, KEY_ENTER},
};
#define MAPNORMAL_2_KEYCODE_COUNT (sizeof(mapNormal_2_KeyCode)/sizeof(mapNormal_2_KeyCode[0]))

static CSI_F_2_Key mapCSI_F_2_KeyCode[] = {
    { 'A', KEY_UP},
    { 'B', KEY_DOWN},
    { 'C', KEY_RIGHT},
    { 'D', KEY_LEFT},
};
#define MAPCSI_F_2_KEYCODE_COUNT (sizeof(mapCSI_F_2_KeyCode)/sizeof(mapCSI_F_2_KeyCode[0]))

static int kbd_select1[]  = {KEY_1, 0};
static int kbd_select2[]  = {KEY_2, 0};
static int kbd_select3[]  = {KEY_3, 0};
static int kbd_select4[]  = {KEY_4, 0};
static int kbd_select5[]  = {KEY_5, 0};
static int kbd_select6[]  = {KEY_6, 0};
static int kbd_select7[]  = {KEY_7, 0};
static int kbd_select8[]  = {KEY_8, 0};
static int kbd_select9[]  = {KEY_9, 0};
static int kbd_select10[] = {KEY_0, 0};
static int kbd_left[]     = {KEY_LEFT,  0};
static int kbd_right[]    = {KEY_RIGHT, 0};
static int kbd_up[]       = {KEY_UP,   0};
static int kbd_down[]     = {KEY_DOWN,  0};
static int kbd_select[]   = {KEY_ENTER, KEY_SPACE, 0};
static int kbd_abort[]    = {KEY_ESC, KEY_Q, 0};

static event_map kbd_event_map = {NULL, 
                                  kbd_select1,
                                  kbd_select2,
                                  kbd_select3,
                                  kbd_select4,
                                  kbd_select5,
                                  kbd_select6,
                                  kbd_select7,
                                  kbd_select8,
                                  kbd_select9,
                                  kbd_select10,
                                  kbd_left,
                                  kbd_right,
                                  kbd_up,
                                  kbd_down,
                                  kbd_select,
                                  kbd_abort};


static int16_t map2keyCode(const CSI_F_2_Key *map,  size_t size, int8_t stdIn)
{
    int16_t keyCode = KEY_RESERVED;
    int i;

    for (i = 0; i < size; ++i)
    {
        if (map[i].stdIn == stdIn)
        {
            keyCode = map[i].keyCode;
            break;
        }
    }

    return keyCode;
}


#define normalMap2keyCode(stdIn) map2keyCode(mapNormal_2_KeyCode, MAPNORMAL_2_KEYCODE_COUNT, stdIn)
#define csi_F_Map2keyCode(stdIn) map2keyCode(mapCSI_F_2_KeyCode,  MAPCSI_F_2_KEYCODE_COUNT,  stdIn)


static void readNextStdIn()
{
    uint8_t  keyIn;
    uint16_t keyOut;
    ssize_t retval;

    retval = read(STDIN_FILENO, &keyIn, sizeof(keyIn));

    if (retval > 0)
    {
        switch (curCSISeqState)
        {
        case CSI_none:
            if (keyIn == CSI_1)
            {
                curCSISeqState = CSI_ESCread;
                getCurClock(&startCSI);
            }
            else
            {
                keyOut = normalMap2keyCode(keyIn);
                if (keyOut != KEY_RESERVED)
                {
                    write(kbdPipe[1], &keyOut, sizeof(keyOut));
                }
            }
            break;
        case CSI_ESCread:
            if (keyIn == CSI_2)
            {
                curCSISeqState = CSI_waitForEnd;
            }
            else
            {
                keyOut = KEY_ESC;
                write(kbdPipe[1], &keyOut, sizeof(keyOut));
                if (keyIn != CSI_1)
                {
                    keyOut = normalMap2keyCode(keyIn);;
                    if (keyOut != KEY_RESERVED)
                    {
                        write(kbdPipe[1], &keyOut, sizeof(keyOut));
                    }
                    curCSISeqState = CSI_none;
                }
            }
            break;
        case CSI_waitForEnd:
            if ((keyIn >= CSI_F_FIRST) && (keyIn <= CSI_F_LAST))
            {
                keyOut = csi_F_Map2keyCode(keyIn);
                if (keyOut != KEY_RESERVED)
                {
                    write(kbdPipe[1], &keyOut, sizeof(keyOut));
                }
                curCSISeqState = CSI_none;
            }
            break;
        }
    }
    else
    {
        debugOut(debug_level2, "KBD error stdin\n");
    }
}


int kbd_init()
{
    if (!init)
    {
        int retval;

        curCSISeqState = CSI_none;
        retval = pipe(kbdPipe);

        if(retval)
        {
            return -1;
        }

        init = 1;

        return 0;
    }
    else
    {
        return -1;
    }
}


void kbd_task()
{
    if (init)
    {
        if (curCSISeqState != CSI_none)
        {
            int16_t keyOut = KEY_ESC;
            write(kbdPipe[1], &keyOut, sizeof(keyOut));
            curCSISeqState = CSI_none;
        }
    }
}


int kbd_getTaskTimeout()
{
    if (init && (curCSISeqState != CSI_none))
    {
        int to = getTimeout(&startCSI, escTimeout);
        if (to < 0) { kbd_task(); }
        return to;
    }
    else
    {
        return -1;
    }
}


void kbd_stop()
{
    if (init)
    {
        close(kbdPipe[1]);
        close(kbdPipe[0]);
        kbdPipe[1] = kbdPipe[0] = -1;
        init = 0;
    }
}


int kbd_getFd(int devNr)
{
    switch (devNr)
    {
    case 0:
        return STDIN_FILENO;
    case 1:
        return kbdPipe[0];
    default:
        return -1;
    }
}


input_event kbd_getEvent(int devNr)
{

    switch (devNr)
    {
    case 0:
        readNextStdIn();
        return input_none;
    case 1:
    {
        uint16_t keyCode;
        ssize_t retval;
        retval = read(kbdPipe[0], &keyCode, sizeof(keyCode));
        if (retval == sizeof(keyCode))
        {
            return key2event(kbd_event_map, keyCode);
        }
        else
        {
            return input_none;
        }
    }
    default:
        return input_none;
    }

}
