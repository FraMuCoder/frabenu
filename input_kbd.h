/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#ifndef _FRABENU_INPUT_KBD_H_
#define _FRABENU_INPUT_KBD_H_

#include "input.h"

#define INPUT_KBD_MAX 2

/**
 * Init input logic.
 *
 * Call this only once or after calling kbd_stop().
 *
 * @return Return 0 on success and a value != 0 on error.
 *         You should call kbd_stop() even if init fail.
 */
int kbd_init();

/**
 * @brief Task to handle timeouts, monitoring or else.
 *
 * Currently you do not call this directly.
 */
void kbd_task();

/**
 * @brief Get timeout until next task call is needed or -1.
 *
 * If timeout elapsed, kbd_task() is called.
 * Use the return vale for blocking functions with timeout.
 * @return Timeout in ms or -1 if no timeout is running.
 */
int  kbd_getTaskTimeout();

/**
 * Deinit input logic.
 *
 * Call this even if init fail to deinit partly initialization.
 * kbd_stop() do nothing if kbd_init() is not called before.
 */
void kbd_stop();

/**
 * @brief Get file descriptor for a device
 * @param devNr 0..(INPUT_KBD_MAX-1)
 * @return      >=0 for open device or -1 if device is closed.
 */
int kbd_getFd(int devNr);

/**
 * @brief Read data from open device and return current input event.
 *
 * This may block if there are no data to read.
 * @param devNr 0..(INPUT_KBD_MAX-1)
 * @return      Current input event, may be input_none.
 */
input_event kbd_getEvent(int devNr);

#endif // _FRABENU_INPUT_KBD_H_
