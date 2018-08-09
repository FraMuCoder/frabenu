/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#ifndef _FRABENU_GLOBALS_H_
#define _FRABENU_GLOBALS_H_

typedef enum
{
    debug_level0, // always
    debug_level1,
    debug_level2,
    debug_level3
} debug_level;

void setDebugLevel(debug_level lev);

void debugOut(debug_level veb, const char * str, ...);

#endif // _FRABENU_GLOBALS_H_
