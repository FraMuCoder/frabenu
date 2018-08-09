/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#include "../menu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define ASSERT_EX(expr, ex)     if (!(expr)) \
                                { \
                                    ++err;\
                                    fprintf(stderr, "ASSERT %s:%d\n", __FILE__, __LINE__); \
                                    ex; \
                                }

#define ASSERT(expr)            ASSERT_EX(expr, )

#define ASSERT_INTEQ(i1, i2)    ASSERT_EX(i1 == i2, fprintf(stderr, "\t%d != %d\n", i1, i2)); \

#define ASSERT_STREQ(s1, s2)    ASSERT_EX(strcmp(s1, s2) == 0, fprintf(stderr, "\t\"%s\" != \"%s\"\n", s1, s2))

#define ASSERT_PTREQ(p1, p2)    ASSERT_EX(p1 == p2, fprintf(stderr, "\t\0x%x != 0x%x\n", p1, p2))

extern char *buildFileName(const char *fileName, uint8_t *xPos, uint8_t *yPos);

int test_menu_buildFileName()
{
    int err = 0;
                 // 0123456789012
    char fn1_i[] = "Test%x_%y.png";
    char fn1_o[] = "Test%_%.png";
    char fn2_o[] = "Test%_%y.png";
    char fn3_o[] = "Test%x_%.png";
    char fn4_i[] = "Test%y_%x.png";
    uint8_t xPos;
    uint8_t yPos;
    char *res;

    xPos = 1;
    yPos = 1;
    res = buildFileName(fn1_i, &xPos, &yPos);
    ASSERT(res != NULL);
    ASSERT_INTEQ(xPos, 4);
    ASSERT_INTEQ(yPos, 6);
    ASSERT_STREQ(res, fn1_o);
    free(res);

    xPos = 1;
    yPos = 0;
    res = buildFileName(fn1_i, &xPos, &yPos);
    ASSERT(res != NULL);
    ASSERT_INTEQ(xPos, 4);
    ASSERT_INTEQ(yPos, 0);
    ASSERT_STREQ(res, fn2_o);
    free(res);

    xPos = 0;
    yPos = 1;
    res = buildFileName(fn1_i, &xPos, &yPos);
    ASSERT(res != NULL);
    ASSERT_INTEQ(xPos, 0);
    ASSERT_INTEQ(yPos, 7);
    ASSERT_STREQ(res, fn3_o);
    free(res);

    xPos = 1;
    yPos = 1;
    res = buildFileName(fn4_i, &xPos, &yPos);
    ASSERT(res != NULL);
    ASSERT_INTEQ(xPos, 6);
    ASSERT_INTEQ(yPos, 4);
    ASSERT_STREQ(res, fn1_o);
    free(res);

    return err;
}

int test_menu_creat()
{
    int err = 0;
    menu *m;

    uint8_t xMax;
    uint8_t yMax;
    char fn[] = "menu_%x_%y.png";

    xMax = 3;
    yMax = 2;
    m = menu_creat(xMax, yMax, fn);
    ASSERT(m != NULL);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    m = menu_destroy(m);
    ASSERT(m == NULL);

    return err;
}

int test_menu_img()
{
    int err = 0;
    struct ida_image * img;
    menu *m;
    uint8_t xMax = 3;
    uint8_t yMax = 2;
    char fn[] = "menu_%x_%y.png";

    m = menu_creat(xMax, yMax, fn);
    ASSERT(m != NULL);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    img = menu_img(m);
    ASSERT(img == m->imgArr[0]);

    m->curX = 1;
    m->curY = 0;
    img = menu_img(m);
    ASSERT(img == m->imgArr[1]);

    m->curX = 2;
    m->curY = 0;
    img = menu_img(m);
    ASSERT(img == m->imgArr[2]);

    m->curX = 0;
    m->curY = 1;
    img = menu_img(m);
    ASSERT(img == m->imgArr[3]);

    m->curX = 1;
    m->curY = 1;
    img = menu_img(m);
    ASSERT(img == m->imgArr[4]);

    m->curX = 2;
    m->curY = 1;
    img = menu_img(m);
    ASSERT(img == m->imgArr[5]);

    m = menu_destroy(m);
    ASSERT(m == NULL);

    return err;
}

int test_menu_task_m1()
{
    int err = 0;
    int ret;
    menu *m;
    uint8_t xMax = 3;
    uint8_t yMax = 2;
    char fn[] = "menu_%x_%y.png";
    menu_scroll_mode mode = menu_scroll_mode_1;

    m = menu_creat(xMax, yMax, fn);
    ASSERT(m != NULL);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    m = menu_destroy(m);
    ASSERT(m == NULL);

    return err;
}

int test_menu_task_m2()
{
    int err = 0;
    int ret;
    menu *m;
    uint8_t xMax = 3;
    uint8_t yMax = 2;
    char fn[] = "menu_%x_%y.png";
    menu_scroll_mode mode = menu_scroll_mode_2;

    m = menu_creat(xMax, yMax, fn);
    ASSERT(m != NULL);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    m = menu_destroy(m);
    ASSERT(m == NULL);

    return err;
}

int test_menu_task_m3()
{
    int err = 0;
    int ret;
    menu *m;
    uint8_t xMax = 3;
    uint8_t yMax = 2;
    char fn[] = "menu_%x_%y.png";
    menu_scroll_mode mode = menu_scroll_mode_3;

    m = menu_creat(xMax, yMax, fn);
    ASSERT(m != NULL);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    m = menu_destroy(m);
    ASSERT(m == NULL);

    return err;
}

int test_menu_task_m4()
{
    int err = 0;
    int ret;
    menu *m;
    uint8_t xMax = 3;
    uint8_t yMax = 2;
    char fn[] = "menu_%x_%y.png";
    menu_scroll_mode mode = menu_scroll_mode_4;

    m = menu_creat(xMax, yMax, fn);
    ASSERT(m != NULL);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_right);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_down);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_left);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_up);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    m = menu_destroy(m);
    ASSERT(m == NULL);

    return err;
}

int test_menu_task_select()
{
    int err = 0;
    int ret;
    menu *m;
    uint8_t xMax = 3;
    uint8_t yMax = 2;
    char fn[] = "menu_%x_%y.png";
    menu_scroll_mode mode = menu_scroll_mode_1;

    m = menu_creat(xMax, yMax, fn);
    ASSERT(m != NULL);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_select1);
    ASSERT_INTEQ(ret, 1);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_select2);
    ASSERT_INTEQ(ret, 2);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_select3);
    ASSERT_INTEQ(ret, 3);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 0);

    ret = menu_task(m, mode, input_select4);
    ASSERT_INTEQ(ret, 4);
    ASSERT_INTEQ(m->curX, 0);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_select5);
    ASSERT_INTEQ(ret, 5);
    ASSERT_INTEQ(m->curX, 1);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_select6);
    ASSERT_INTEQ(ret, 6);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_select7);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_select8);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_select9);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_select10);
    ASSERT_INTEQ(ret, -1);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_select);
    ASSERT_INTEQ(ret, 6);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    ret = menu_task(m, mode, input_abort);
    ASSERT_INTEQ(ret, 0);
    ASSERT_INTEQ(m->curX, 2);
    ASSERT_INTEQ(m->curY, 1);

    m = menu_destroy(m);
    ASSERT(m == NULL);

    return err;
}

int main(int argc, char **argv)
{
    int err = 0;

    err += test_menu_buildFileName();

    err += test_menu_creat();

    err += test_menu_img();

    err += test_menu_task_m1();

    err += test_menu_task_m2();

    err += test_menu_task_m3();

    err += test_menu_task_m4();

    err += test_menu_task_select();

    return err;
}
