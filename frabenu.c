/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "input.h"
#include "menu.h"
#include "fbida/fbi.h"
#include "fbida/fbtools.h"
#include "fbida/fb-gui.h"
#include "fbida/vt.h"
#include "fbida/kbd.h"


/* graphics interface */
gfxstate                   *gfx;


menu_scroll_mode scrollMode = menu_scroll_mode_1;
char *fileName = NULL;
uint8_t xMax = 1, yMax = 1;


static jmp_buf fb_fatal_cleanup;


static void catch_exit_signal(int signal)
{
    siglongjmp(fb_fatal_cleanup,signal);
}


static void exit_signals_init(void)
{
    struct sigaction act,old;
    int termsig;

    memset(&act,0,sizeof(act));
    act.sa_handler = catch_exit_signal;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act,&old);
    sigaction(SIGQUIT,&act,&old);
    sigaction(SIGTERM,&act,&old);

    sigaction(SIGABRT,&act,&old);
    sigaction(SIGTSTP,&act,&old);

    sigaction(SIGBUS, &act,&old);
    sigaction(SIGILL, &act,&old);
    sigaction(SIGSEGV,&act,&old);

    if (0 == (termsig = sigsetjmp(fb_fatal_cleanup,0)))
        return;

    /* cleanup */
    gfx->cleanup_display();
    console_switch_cleanup();
    input_stop();
    debugOut(debug_level0, "Oops: %s\n", strsignal(termsig));
    exit(-1);
}


static void cleanup_and_exit(int code)
{
    shadow_fini();
    tty_restore();
    gfx->cleanup_display();
    input_stop();
    console_switch_cleanup();
    exit(code);
}


static void console_switch_redraw(void)
{
    gfx->restore_display();
    shadow_set_dirty();
    shadow_render(gfx);
}


int parseArgs(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "hs:")) != -1)
    {
        switch (opt)
        {
        case 's':
            if (optarg != NULL)
            {
                if ((optarg[0] >= '1') && (optarg[0] <= '4') && (optarg[1] == 0))
                {
                    scrollMode = optarg[0] - '1';
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }
            break;
        case 'h':
        case '?':
        default:
            return -1;
        }
    }

    if ((argc - optind) == 2)
    {
        if (    (argv[optind][0] >= '1') && (argv[optind][0] <= '9') &&
                (argv[optind][1] == 'x') &&
                (argv[optind][2] >= '1') && (argv[optind][2] <= '9') &&
                (argv[optind][3] == 0))
        {
            xMax = argv[optind][0] - '0';
            yMax = argv[optind][2] - '0';
        }
        else
        {
            return -1;
        }

        fileName = argv[optind+1];

        return 0;
    }
    else
    {
        return -1;
    }
}


int main(int argc, char **argv)
{
    int vt = 0;
    char *videoMode = NULL;
    menu * m;
    int select = -1;
    input_event event;

    setDebugLevel(debug_level0);

    if (0 != parseArgs(argc, argv))
    {
        return -1;
    }

    m = menu_creat(xMax, yMax, fileName);
    if (m == NULL) { return -1; }

    input_init();
    gfx = fb_init(NULL, videoMode, vt);

    exit_signals_init();
    signal(SIGTSTP,SIG_IGN);

    if (console_switch_init(console_switch_redraw) < 0) {
        debugOut(debug_level0, "NOTICE: No vt switching available on terminal.\n");
    }
    shadow_init(gfx);

    tty_raw();

    while (select < 0)
    {
        shadow_draw_image(gfx, menu_img(m), 0, 0, 0, gfx->vdisplay-1, 100);
        shadow_render(gfx);

        event = input_get();
        select = menu_task(m, scrollMode, event);
    }

    menu_destroy(m);

    cleanup_and_exit(select);

    return select;
}
