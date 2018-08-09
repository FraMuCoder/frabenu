/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#ifndef _FRABENU_INPUT_JOY_H_
#define _FRABENU_INPUT_JOY_H_

#include "input.h"

#define INPUT_JOY_MAX 10

/**
 * @brief Add new joystick device for monitoring.
 *
 * You must call this before joy_init(). If yo do not call this, default devies are used.
 * @param devName   Joystick device e.g. "/dev/input/js0".
 * @return Returns 0 on success and a value != 0 on error.
 */
int joy_cfgAddDev(const char *devName);

/**
 * Init input logic.
 *
 * Call this only once or after calling joy_stop().
 *
 * @return Returns 0 on success and a value != 0 on error.
 *         You should call joy_stop() even if init fail.
 */
int joy_init();

/**
 * @brief Task to handle timeouts, monitoring or else.
 *
 * Currently you do not call this directly.
 */
void joy_task();

/**
 * @brief Get timeout until next task call is needed or -1.
 *
 * If timeout elapsed, joy_task() is called.
 * Use the return vale for blocking functions with timeout.
 * @return Timeout in ms or -1 if no timeout is running.
 */
int  joy_getTaskTimeout();

/**
 * Deinit input logic.
 *
 * Call this even if init fail to deinit partly initialization.
 * joy_stop() do nothing if joy_init() is not called before.
 */
void joy_stop();

/**
 * @brief Get file descriptor for a device
 * @param devNr 0..(INPUT_JOY_MAX-1)
 * @return      >=0 for open device or -1 if device is closed.
 */
int joy_getFd(int devNr);

/**
 * @brief Read data from open device and return current input event.
 *
 * This may block if there are no data to read.
 * @param devNr 0..(INPUT_JOY_MAX-1)
 * @return      Current input event, may be input_none.
 */
input_event joy_getEvent(int devNr);

#endif // _FRABENU_INPUT_JOY_H_
