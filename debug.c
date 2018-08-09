/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

static debug_level level = debug_level1;

void setDebugLevel(debug_level lev)
{
    level = lev;
}


void debugOut(debug_level lev, const char * str, ...)
{
    if (lev <= level)
    {
        static char tmp[512];
        int retval;
        va_list ap;

        va_start(ap, str);
        retval = vsnprintf(tmp, sizeof(tmp), str, ap);
        va_end(ap);

        if (retval > 0)
        {
            fputs(tmp, stderr);
        }
    }
}
