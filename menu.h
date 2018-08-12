/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#ifndef _FRABENU_MENU_H_
#define _FRABENU_MENU_H_

#include "input.h"
#include <stdint.h>

typedef struct menu
{
    int8_t xMax;
    int8_t yMax;
    int8_t curX;    // cur marker
    int8_t curY;
    struct ida_image **imgArr;
} menu;

typedef enum menu_scroll_mode
{
    menu_scroll_mode_1, // stop at borders
    menu_scroll_mode_2, // roll through x/y independent
    menu_scroll_mode_3, // roll through all, stop at 1st and last
    menu_scroll_mode_4  // roll through all
} menu_scroll_mode;

/**
 * @brief menu_creat
 * @param xMax  1..9
 * @param yMax  1..9
 * @param fileName something like "menu_%x_%y.png"
 * @return      New menu struct or NULL on error.
 */
menu *menu_creat(uint8_t xMax, uint8_t yMax, const char *fileName);

/**
 * @brief menu_destroy
 * @param m     Pointer of menu struct on heap to free
 * @return      NULL
 */
menu *menu_destroy(menu * m);

/**
 * @brief Get marked index.
 * @param m
 * @return      1..(xMax*yMax-1), -1 on error
 */
int menu_get(menu * m);

/**
 * @brief Set menu selection.
 * @param m
 * @param select    new seletion 1..(xMax*yMax-1)
 */
void menu_set(menu *m , int select);

/**
 * @brief Get currenly marked image.
 * @param m
 * @return      Image or NULL on error.
 */
struct ida_image * menu_img(menu * m);

/**
 * @brief Handle input event.
 * @param m
 * @param mode  Scroll mode
 * @param e     Input event
 * @return      <0 : currently nothing selected
 *               0 : menu abort
 *               1..(xMax*yMax-1) : selected menu index
 */
int menu_task(menu * m, menu_scroll_mode mode, input_event e);


#endif // _FRABENU_MENU_H_
