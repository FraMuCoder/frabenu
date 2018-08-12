/* *******************************************
 * frabenu - Framebuffer menu
 * Copyright (C) 2018 Frank Mueller
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * *******************************************/

#include "menu.h"
#include "fbida/fbi.h"
#include <string.h>
#include <stdlib.h>
#include "debug.h"

/**menu
 * @brief buildFileName
 * @param fileName
 * @param[in,out] xPos
 * @param[in,out] yPos
 * @return
 */
char *buildFileName(const char *fileName, uint8_t *xPos, uint8_t *yPos)
{
    int i;
    int first, second;
    int x, y;
    int cnt, needed;
    int len;
    char *str;

    if (fileName == NULL) { return NULL; }

    y = x = -1;
    needed = 0;
    if (*xPos > 0) { ++needed; }
    if (*yPos > 0) { ++needed; }

    if (needed == 0) { return NULL; }

    for (i = 0, cnt = needed; (fileName[i] != 0) && (cnt > 0); ++i)
    {
        if (fileName[i] == '%')
        {
            if ((*xPos > 0) && (x < 0) && (fileName[i+1] == 'x'))        // need x and x not yet found and x now found
            {
                x = i;
                ++i;
                --cnt;
            }
            else if ((*yPos > 0) && (y < 0) && (fileName[i+1] == 'y'))   // need y and y not yet found and y now found
            {
                y = i;
                ++i;
                --cnt;
            }
        }
    }

    if (cnt > 0) { return NULL; } // not all found

    len = strlen(fileName) - needed + 1;
    str = malloc(len);
    if (str == NULL) { return NULL; }

    if (needed == 2)
    {
        if (x < y)
        {
            first = x;
            second = y;
            *xPos = x;
            *yPos = y-1;
        }
        else
        {
            first = y;
            second = x;
            *xPos = x-1;
            *yPos = y;
        }
    }
    else if (*xPos > 0)
    {
        first = x;
        second = -1;
        *xPos = x;
    }
    else
    {
        first = y;
        second = -1;
        *yPos = y;
    }
    strncpy(str, fileName, first+1);

    if (second > 0)
    {
        strncpy(str+first+1, fileName+first+2, second-first-1);
        strncpy(str+second, fileName+second+2, len-second);
    }
    else
    {
        strncpy(str+first+1, fileName+first+2, len-first-1);
    }

    return str;
}


menu *menu_creat(uint8_t xMax, uint8_t yMax, const char *fileName)
{
    menu *m;
    uint8_t xPos;
    uint8_t yPos;
    uint8_t x, y;
    char *str;
    struct ida_image *img;

    debugOut(debug_level3, "menu_creat(%d, %d, %s)\n", xMax, yMax, fileName);

    if ((xMax < 1) || (xMax > 9) || (yMax < 1) || (yMax > 9)) { return NULL; }

    m = calloc(1, sizeof(menu));
    if (m == NULL) { return NULL; }

    m->curX = m->curY = 0;
    m->xMax = xMax;
    m->yMax = yMax;

    m->imgArr = calloc(xMax * yMax, sizeof(m->imgArr));

    xPos = (xMax == 1) ? 0 : 1;
    yPos = (yMax == 1) ? 0 : 1;
    str = buildFileName(fileName, &xPos, &yPos);
    if (str == NULL) { return menu_destroy(m); }

    for (y = 0; y < yMax; ++y)
    {
        for (x = 0; x < xMax; ++x)
        {
            if (xPos > 0) { str[xPos] = '0' + x + 1; }
            if (yPos > 0) { str[yPos] = '0' + y + 1; }
            debugOut(debug_level3, "try read %s\n", str);
            img = read_image(str);
            if (img == NULL)
            {
                free(str);
                return menu_destroy(m);
            }
            m->imgArr[y*xMax + x] = img;
        }
    }

    free(str);

    return m;
}


menu *menu_destroy(menu * m)
{
    if (m != NULL)
    {
        if (m->imgArr != NULL)
        {
            uint8_t x, y;
            for (y = 0; y < m->yMax; ++y)
            {
                for (x = 0; x < m->xMax; ++x)
                {
                    if (m->imgArr[y*m->xMax + x] != NULL)
                    {
                        free(m->imgArr[y*m->xMax + x]);
                    }
                }
            }
            free(m->imgArr);
        }
        free(m);
    }
    return NULL;
}


int menu_get(menu *m)
{
    if (m == NULL) { return -1; }
    return m->curY * m->xMax + m->curX;
}

void menu_set(menu *m , int select)
{
    if ((m == NULL) || (select <= 0)) { return; }
    --select;
    int y = select / m->xMax;
    if (y < m->yMax)
    {
        m->curX = select % m->xMax;
        m->curY = y;
    }
}

struct ida_image * menu_img(menu * m)
{
    if (m == NULL) { return NULL; }
    if (m->imgArr == NULL) { return NULL; }
    return m->imgArr[menu_get(m)];
}


int menu_task(menu * m, menu_scroll_mode mode, input_event e)
{
    int select = -1;
    if (m == NULL) { return 0; }

    switch (e)
    {
    case input_select1:
    case input_select2:
    case input_select3:
    case input_select4:
    case input_select5:
    case input_select6:
    case input_select7:
    case input_select8:
    case input_select9:
    case input_select10:
        if (e <= (m->xMax * m->yMax))
        {
            m->curX = (e-1) % m->xMax;
            m->curY = (e-1) / m->xMax;
            select = e;
        }
        break;
    case input_left:
        if (m->curX == 0)
        {
            switch (mode)
            {
            case menu_scroll_mode_1:
                break;
            case menu_scroll_mode_2:
                m->curX = m->xMax - 1;
                break;
            case menu_scroll_mode_3:
                if (m->curY > 0)
                {
                    m->curX = m->xMax - 1;
                    --m->curY;
                }
                break;
            case menu_scroll_mode_4:
                m->curX = m->xMax - 1;
                if (m->curY > 0)
                {
                    --m->curY;
                }
                else
                {
                    m->curY = m->yMax - 1;
                }
                break;
            }
        }
        else
        {
            --m->curX;
        }
        break;
    case input_right:
        ++m->curX;
        if (m->curX >= m->xMax)
        {
            switch (mode)
            {
            case menu_scroll_mode_1:
                m->curX = m->xMax - 1;
                break;
            case menu_scroll_mode_2:
                m->curX = 0;
                break;
            case menu_scroll_mode_3:
                if (m->curY+1 < m->yMax)
                {
                    m->curX = 0;
                    ++m->curY;
                }
                else
                {
                    m->curX = m->xMax - 1;
                }
                break;
            case menu_scroll_mode_4:
                m->curX = 0;
                if (m->curY+1 < m->yMax)
                {
                    ++m->curY;
                }
                else
                {
                    m->curY = 0;
                }
                break;
            }
        }
        break;
    case input_up:
        if (m->curY == 0)
        {
            switch (mode)
            {
            case menu_scroll_mode_1:
                break;
            case menu_scroll_mode_2:
                m->curY = m->yMax - 1;
                break;
            case menu_scroll_mode_3:
                if (m->curX > 0)
                {
                    m->curY = m->yMax - 1;
                    --m->curX;
                }
                break;
            case menu_scroll_mode_4:
                m->curY = m->yMax - 1;
                if (m->curX > 0)
                {
                    --m->curX;
                }
                else
                {
                    m->curX = m->xMax - 1;
                }
                break;
            }
        }
        else
        {
            --m->curY;
        }
        break;
    case input_down:
        ++m->curY;
        if (m->curY >= m->yMax)
        {
            switch (mode)
            {
            case menu_scroll_mode_1:
                m->curY = m->yMax - 1;
                break;
            case menu_scroll_mode_2:
                m->curY = 0;
                break;
            case menu_scroll_mode_3:
                if (m->curX+1 < m->xMax)
                {
                    m->curY = 0;
                    ++m->curX;
                }
                else
                {
                    m->curY = m->yMax - 1;
                }
                break;
            case menu_scroll_mode_4:
                m->curY = 0;
                if (m->curX+1 < m->xMax)
                {
                    ++m->curX;
                }
                else
                {
                    m->curX = 0;
                }
                break;
            }
        }
        break;
    case input_select:
        select = menu_get(m) + 1;
        break;
    case input_abort:
        select = 0;
        break;
    default:
        break;
    }

    return select;
}
