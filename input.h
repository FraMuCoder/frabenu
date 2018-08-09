/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#ifndef _FRABENU_INPUT_H_
#define _FRABENU_INPUT_H_

typedef enum
{
    input_none,
    input_select1,  // direct select menu item 1
    input_select2,
    input_select3,
    input_select4,
    input_select5,
    input_select6,
    input_select7,
    input_select8,
    input_select9,
    input_select10,
    input_left,
    input_right,
    input_up,
    input_down,
    input_select,
    input_abort,

    input_event_cnt
} input_event;


typedef int * event_map[input_event_cnt];


/**
 * Init input logic.
 *
 * Call this only once or after calling input_stop().
 *
 * @return Return 0 on success and a value != 0 on error.
 *         You should call input_stop() even if init fail.
 */
int input_init(void);

/**
 * Deinit input logic.
 *
 * Call this even if init fail to deinit partly initialization.
 * input_stop() do nothing if input_init() is not called before.
 */
void input_stop(void);

/**
 * Wait on next input event and return it.
 *
 * Currently this also returns after a timeout (1s)
 * to give you a change to poll something.
 *
 * @return Current input event or input_none on timeout.
 */
input_event input_get(void);


/**
 * Helper function to map a key code to an input event.
 *
 * @param map   Map used for the conversion, last element must be a 0.
 * @param key   key code, see linux/input.h
 * @return      Matching event from map or input_none
 */
input_event key2event(const event_map map, int key);

#endif // _FRABENU_INPUT_H_
