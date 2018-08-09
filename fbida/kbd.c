#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <termios.h>

#include "kbd.h"

/* ---------------------------------------------------------------------- */

struct termctrl {
    const char *seq;
    uint32_t code;
    uint32_t mod;
};

static struct termctrl termctrl[] = {
    { .seq = "a", .code = KEY_A },
    { .seq = "b", .code = KEY_B },
    { .seq = "c", .code = KEY_C },
    { .seq = "d", .code = KEY_D },
    { .seq = "e", .code = KEY_E },
    { .seq = "f", .code = KEY_F },
    { .seq = "g", .code = KEY_G },
    { .seq = "h", .code = KEY_H },
    { .seq = "i", .code = KEY_I },
    { .seq = "j", .code = KEY_J },
    { .seq = "k", .code = KEY_K },
    { .seq = "l", .code = KEY_L },
    { .seq = "m", .code = KEY_M },
    { .seq = "n", .code = KEY_N },
    { .seq = "o", .code = KEY_O },
    { .seq = "p", .code = KEY_P },
    { .seq = "q", .code = KEY_Q },
    { .seq = "r", .code = KEY_R },
    { .seq = "s", .code = KEY_S },
    { .seq = "t", .code = KEY_T },
    { .seq = "u", .code = KEY_U },
    { .seq = "v", .code = KEY_V },
    { .seq = "w", .code = KEY_W },
    { .seq = "x", .code = KEY_X },
    { .seq = "y", .code = KEY_Y },
    { .seq = "z", .code = KEY_Z },

    { .seq = "A", .code = KEY_A, .mod = KEY_MOD_SHIFT },
    { .seq = "B", .code = KEY_B, .mod = KEY_MOD_SHIFT },
    { .seq = "C", .code = KEY_C, .mod = KEY_MOD_SHIFT },
    { .seq = "D", .code = KEY_D, .mod = KEY_MOD_SHIFT },
    { .seq = "E", .code = KEY_E, .mod = KEY_MOD_SHIFT },
    { .seq = "F", .code = KEY_F, .mod = KEY_MOD_SHIFT },
    { .seq = "G", .code = KEY_G, .mod = KEY_MOD_SHIFT },
    { .seq = "H", .code = KEY_H, .mod = KEY_MOD_SHIFT },
    { .seq = "I", .code = KEY_I, .mod = KEY_MOD_SHIFT },
    { .seq = "J", .code = KEY_J, .mod = KEY_MOD_SHIFT },
    { .seq = "K", .code = KEY_K, .mod = KEY_MOD_SHIFT },
    { .seq = "L", .code = KEY_L, .mod = KEY_MOD_SHIFT },
    { .seq = "M", .code = KEY_M, .mod = KEY_MOD_SHIFT },
    { .seq = "N", .code = KEY_N, .mod = KEY_MOD_SHIFT },
    { .seq = "O", .code = KEY_O, .mod = KEY_MOD_SHIFT },
    { .seq = "P", .code = KEY_P, .mod = KEY_MOD_SHIFT },
    { .seq = "Q", .code = KEY_Q, .mod = KEY_MOD_SHIFT },
    { .seq = "R", .code = KEY_R, .mod = KEY_MOD_SHIFT },
    { .seq = "S", .code = KEY_S, .mod = KEY_MOD_SHIFT },
    { .seq = "T", .code = KEY_T, .mod = KEY_MOD_SHIFT },
    { .seq = "U", .code = KEY_U, .mod = KEY_MOD_SHIFT },
    { .seq = "V", .code = KEY_V, .mod = KEY_MOD_SHIFT },
    { .seq = "W", .code = KEY_W, .mod = KEY_MOD_SHIFT },
    { .seq = "X", .code = KEY_X, .mod = KEY_MOD_SHIFT },
    { .seq = "Y", .code = KEY_Y, .mod = KEY_MOD_SHIFT },
    { .seq = "Z", .code = KEY_Z, .mod = KEY_MOD_SHIFT },

    { .seq = "\x01", .code = KEY_A, .mod = KEY_MOD_CTRL },
    { .seq = "\x02", .code = KEY_B, .mod = KEY_MOD_CTRL },
    { .seq = "\x03", .code = KEY_C, .mod = KEY_MOD_CTRL },
    { .seq = "\x04", .code = KEY_D, .mod = KEY_MOD_CTRL },
    { .seq = "\x05", .code = KEY_E, .mod = KEY_MOD_CTRL },
    { .seq = "\x06", .code = KEY_F, .mod = KEY_MOD_CTRL },
    { .seq = "\x07", .code = KEY_G, .mod = KEY_MOD_CTRL },
    { .seq = "\x08", .code = KEY_H, .mod = KEY_MOD_CTRL },
    { .seq = "\x09", .code = KEY_I, .mod = KEY_MOD_CTRL },
    { .seq = "\x0a", .code = KEY_ENTER },
    { .seq = "\x0b", .code = KEY_K, .mod = KEY_MOD_CTRL },
    { .seq = "\x0c", .code = KEY_L, .mod = KEY_MOD_CTRL },
    { .seq = "\x0d", .code = KEY_M, .mod = KEY_MOD_CTRL },
    { .seq = "\x0e", .code = KEY_N, .mod = KEY_MOD_CTRL },
    { .seq = "\x0f", .code = KEY_O, .mod = KEY_MOD_CTRL },
    { .seq = "\x10", .code = KEY_P, .mod = KEY_MOD_CTRL },
    { .seq = "\x11", .code = KEY_Q, .mod = KEY_MOD_CTRL },
    { .seq = "\x12", .code = KEY_R, .mod = KEY_MOD_CTRL },
    { .seq = "\x13", .code = KEY_S, .mod = KEY_MOD_CTRL },
    { .seq = "\x14", .code = KEY_T, .mod = KEY_MOD_CTRL },
    { .seq = "\x15", .code = KEY_U, .mod = KEY_MOD_CTRL },
    { .seq = "\x16", .code = KEY_V, .mod = KEY_MOD_CTRL },
    { .seq = "\x17", .code = KEY_W, .mod = KEY_MOD_CTRL },
    { .seq = "\x18", .code = KEY_X, .mod = KEY_MOD_CTRL },
    { .seq = "\x19", .code = KEY_Y, .mod = KEY_MOD_CTRL },
    { .seq = "\x1a", .code = KEY_Z, .mod = KEY_MOD_CTRL },

    { .seq = "0", .code = KEY_0 },
    { .seq = "1", .code = KEY_1 },
    { .seq = "2", .code = KEY_2 },
    { .seq = "3", .code = KEY_3 },
    { .seq = "4", .code = KEY_4 },
    { .seq = "5", .code = KEY_5 },
    { .seq = "6", .code = KEY_6 },
    { .seq = "7", .code = KEY_7 },
    { .seq = "8", .code = KEY_8 },
    { .seq = "9", .code = KEY_9 },

    { .seq = " ",         .code = KEY_SPACE    },
    { .seq = "\x1b",      .code = KEY_ESC      },
    { .seq = "+",         .code = KEY_KPPLUS   },
    { .seq = "-",         .code = KEY_KPMINUS  },
    { .seq = "\x7f",      .code = KEY_BACKSPACE},

    { .seq = "\x1b[A",    .code = KEY_UP       },
    { .seq = "\x1b[B",    .code = KEY_DOWN     },
    { .seq = "\x1b[C",    .code = KEY_RIGHT    },
    { .seq = "\x1b[D",    .code = KEY_LEFT     },
    { .seq = "\x1b[F",    .code = KEY_END      },
    { .seq = "\x1b[H",    .code = KEY_HOME     },

    { .seq = "\x1b[1~",   .code = KEY_HOME     },
    { .seq = "\x1b[2~",   .code = KEY_INSERT   },
    { .seq = "\x1b[3~",   .code = KEY_DELETE   },
    { .seq = "\x1b[4~",   .code = KEY_END      },
    { .seq = "\x1b[5~",   .code = KEY_PAGEUP   },
    { .seq = "\x1b[6~",   .code = KEY_PAGEDOWN },

    { /* EOF */ }
};

uint32_t kbd_parse(const char *key, uint32_t *mod)
{
    int i;

    for (i = 0; termctrl[i].seq != NULL; i++) {
        if (strcmp(key, termctrl[i].seq) == 0) {
            *mod = termctrl[i].mod;
            return termctrl[i].code;
        }
    }
    *mod = 0;
    return KEY_RESERVED;
}

int kbd_wait(int timeout)
{
    struct timeval limit;
    fd_set set;
    int rc;

    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    limit.tv_sec = timeout;
    limit.tv_usec = 0;
    rc = select(STDIN_FILENO + 1, &set, NULL, NULL,
                timeout ? &limit : NULL);
    return rc;
}

/* ---------------------------------------------------------------------- */

struct termios  saved_attributes;
int             saved_fl;

void tty_raw(void)
{
    struct termios tattr;

    fcntl(STDIN_FILENO, F_GETFL, &saved_fl);
    tcgetattr (0, &saved_attributes);

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    memcpy(&tattr,&saved_attributes,sizeof(struct termios));
    tattr.c_lflag &= ~(ICANON|ECHO);
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}

void tty_restore(void)
{
    fcntl(STDIN_FILENO, F_SETFL, saved_fl);
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}
