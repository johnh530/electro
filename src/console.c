/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    ELECTRO is free software;  you can redistribute it and/or modify it    */
/*    under the terms of the  GNU General Public License  as published by    */
/*    the  Free Software Foundation;  either version 2 of the License, or    */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "opengl.h"
#include "utility.h"
#include "viewport.h"
#include "console.h"
#include "script.h"

#define STEP_X  6
#define STEP_Y 13

/*---------------------------------------------------------------------------*/

GLubyte glyph[96][13] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /*   */
    { 0x00, 0x00, 0x08, 0x00, 0x08, 0x08, 0x08,
      0x08, 0x08, 0x08, 0x08, 0x00, 0x00 }, /* ! */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x14, 0x14, 0x14, 0x00, 0x00 }, /* " */
    { 0x00, 0x00, 0x00, 0x14, 0x14, 0x3e, 0x14,
      0x3e, 0x14, 0x14, 0x00, 0x00, 0x00 }, /* # */
    { 0x00, 0x00, 0x08, 0x3c, 0x0a, 0x0a, 0x1c,
      0x28, 0x28, 0x1e, 0x08, 0x00, 0x00 }, /* $ */
    { 0x00, 0x00, 0x24, 0x2a, 0x14, 0x10, 0x08,
      0x04, 0x14, 0x2a, 0x12, 0x00, 0x00 }, /* % */
    { 0x00, 0x00, 0x1a, 0x24, 0x26, 0x28, 0x10,
      0x28, 0x28, 0x10, 0x00, 0x00, 0x00 }, /* & */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x08, 0x08, 0x08, 0x00, 0x00 }, /* ' */
    { 0x00, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10,
      0x10, 0x10, 0x08, 0x08, 0x04, 0x00 }, /* ( */
    { 0x00, 0x10, 0x08, 0x08, 0x04, 0x04, 0x04,
      0x04, 0x04, 0x08, 0x08, 0x10, 0x00 }, /* ) */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
      0x2a, 0x1c, 0x2a, 0x08, 0x00, 0x00 }, /* * */
    { 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x3e,
      0x08, 0x08, 0x00, 0x00, 0x00, 0x00 }, /* + */
    { 0x00, 0x10, 0x08, 0x0c, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* , */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* - */
    { 0x00, 0x08, 0x1c, 0x08, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* . */
    { 0x00, 0x00, 0x20, 0x20, 0x10, 0x10, 0x08,
      0x04, 0x04, 0x02, 0x02, 0x00, 0x00 }, /* / */
    { 0x00, 0x00, 0x08, 0x14, 0x22, 0x22, 0x22,
      0x22, 0x22, 0x14, 0x08, 0x00, 0x00 }, /* 0 */
    { 0x00, 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08,
      0x08, 0x28, 0x18, 0x08, 0x00, 0x00 }, /* 1 */
    { 0x00, 0x00, 0x3e, 0x20, 0x10, 0x08, 0x04,
      0x02, 0x22, 0x22, 0x1c, 0x00, 0x00 }, /* 2 */
    { 0x00, 0x00, 0x1c, 0x22, 0x02, 0x02, 0x1c,
      0x08, 0x04, 0x02, 0x3e, 0x00, 0x00 }, /* 3 */
    { 0x00, 0x00, 0x04, 0x04, 0x3e, 0x24, 0x14,
      0x14, 0x0c, 0x04, 0x04, 0x00, 0x00 }, /* 4 */
    { 0x00, 0x00, 0x1c, 0x22, 0x02, 0x02, 0x32,
      0x2c, 0x20, 0x20, 0x3e, 0x00, 0x00 }, /* 5 */
    { 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x3c,
      0x20, 0x20, 0x22, 0x1c, 0x00, 0x00 }, /* 6 */
    { 0x00, 0x00, 0x10, 0x10, 0x10, 0x08, 0x08,
      0x04, 0x04, 0x02, 0x3e, 0x00, 0x00 }, /* 7 */
    { 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x1c,
      0x22, 0x22, 0x22, 0x1c, 0x00, 0x00 }, /* 8 */
    { 0x00, 0x00, 0x1c, 0x22, 0x02, 0x02, 0x1e,
      0x22, 0x22, 0x22, 0x1c, 0x00, 0x00 }, /* 9 */
    { 0x00, 0x08, 0x1c, 0x08, 0x00, 0x00, 0x08,
      0x1c, 0x08, 0x00, 0x00, 0x00, 0x00 }, /* : */
    { 0x00, 0x10, 0x08, 0x0c, 0x00, 0x00, 0x08,
      0x1c, 0x08, 0x00, 0x00, 0x00, 0x00 }, /* ; */
    { 0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20,
      0x10, 0x08, 0x04, 0x02, 0x00, 0x00 }, /* < */
    { 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00,
      0x3e, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* = */
    { 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02,
      0x04, 0x08, 0x10, 0x20, 0x00, 0x00 }, /* > */
    { 0x00, 0x00, 0x08, 0x00, 0x08, 0x08, 0x04,
      0x02, 0x22, 0x22, 0x1c, 0x00, 0x00 }, /* ? */
    { 0x00, 0x00, 0x1e, 0x20, 0x2c, 0x2a, 0x2a,
      0x26, 0x22, 0x22, 0x1c, 0x00, 0x00 }, /* @ */
    { 0x00, 0x00, 0x22, 0x22, 0x22, 0x3e, 0x22,
      0x22, 0x22, 0x14, 0x08, 0x00, 0x00 }, /* A */
    { 0x00, 0x00, 0x3c, 0x12, 0x12, 0x12, 0x1c,
      0x12, 0x12, 0x12, 0x3c, 0x00, 0x00 }, /* B */
    { 0x00, 0x00, 0x1c, 0x22, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x22, 0x1c, 0x00, 0x00 }, /* C */
    { 0x00, 0x00, 0x3c, 0x12, 0x12, 0x12, 0x12,
      0x12, 0x12, 0x12, 0x3c, 0x00, 0x00 }, /* D */
    { 0x00, 0x00, 0x3e, 0x20, 0x20, 0x20, 0x3c,
      0x20, 0x20, 0x20, 0x3e, 0x00, 0x00 }, /* E */
    { 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x3c,
      0x20, 0x20, 0x20, 0x3e, 0x00, 0x00 }, /* F */
    { 0x00, 0x00, 0x1c, 0x22, 0x22, 0x26, 0x20,
      0x20, 0x20, 0x22, 0x1c, 0x00, 0x00 }, /* G */
    { 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x3e,
      0x22, 0x22, 0x22, 0x22, 0x00, 0x00 }, /* H */
    { 0x00, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08,
      0x08, 0x08, 0x08, 0x1c, 0x00, 0x00 }, /* I */
    { 0x00, 0x00, 0x18, 0x24, 0x04, 0x04, 0x04,
      0x04, 0x04, 0x04, 0x0e, 0x00, 0x00 }, /* J */
    { 0x00, 0x00, 0x22, 0x22, 0x24, 0x28, 0x30,
      0x28, 0x24, 0x22, 0x22, 0x00, 0x00 }, /* K */
    { 0x00, 0x00, 0x3e, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x00, 0x00 }, /* L */
    { 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x2a,
      0x2a, 0x36, 0x22, 0x22, 0x00, 0x00 }, /* M */
    { 0x00, 0x00, 0x22, 0x22, 0x26, 0x26, 0x2a,
      0x2a, 0x32, 0x32, 0x22, 0x00, 0x00 }, /* N */
    { 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x22,
      0x22, 0x22, 0x22, 0x1c, 0x00, 0x00 }, /* O */
    { 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x3c,
      0x22, 0x22, 0x22, 0x3c, 0x00, 0x00 }, /* P */
    { 0x00, 0x02, 0x1c, 0x2a, 0x22, 0x22, 0x22,
      0x22, 0x22, 0x22, 0x1c, 0x00, 0x00 }, /* Q */
    { 0x00, 0x00, 0x22, 0x22, 0x24, 0x28, 0x3c,
      0x22, 0x22, 0x22, 0x3c, 0x00, 0x00 }, /* R */
    { 0x00, 0x00, 0x1c, 0x22, 0x02, 0x02, 0x1c,
      0x20, 0x20, 0x22, 0x1c, 0x00, 0x00 }, /* S */
    { 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08,
      0x08, 0x08, 0x08, 0x3e, 0x00, 0x00 }, /* T */
    { 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x22,
      0x22, 0x22, 0x22, 0x22, 0x00, 0x00 }, /* U */
    { 0x00, 0x00, 0x08, 0x08, 0x14, 0x14, 0x14,
      0x22, 0x22, 0x22, 0x22, 0x00, 0x00 }, /* V */
    { 0x00, 0x00, 0x14, 0x2a, 0x2a, 0x2a, 0x2a,
      0x22, 0x22, 0x22, 0x22, 0x00, 0x00 }, /* W */
    { 0x00, 0x00, 0x22, 0x22, 0x14, 0x14, 0x08,
      0x14, 0x14, 0x22, 0x22, 0x00, 0x00 }, /* X */
    { 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08,
      0x14, 0x14, 0x22, 0x22, 0x00, 0x00 }, /* Y */
    { 0x00, 0x00, 0x3e, 0x20, 0x10, 0x10, 0x08,
      0x04, 0x04, 0x02, 0x3e, 0x00, 0x00 }, /* Z */
    { 0x00, 0x1c, 0x10, 0x10, 0x10, 0x10, 0x10,
      0x10, 0x10, 0x10, 0x10, 0x1c, 0x00 }, /* [ */
    { 0x00, 0x00, 0x02, 0x02, 0x04, 0x04, 0x08,
      0x10, 0x10, 0x20, 0x20, 0x00, 0x00 }, /* \ */
    { 0x00, 0x1c, 0x04, 0x04, 0x04, 0x04, 0x04,
      0x04, 0x04, 0x04, 0x04, 0x1c, 0x00 }, /* ] */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x22, 0x14, 0x08, 0x00, 0x00 }, /* ^ */
    { 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* _ */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x08, 0x08, 0x08, 0x00, 0x00 }, /* ` */
    { 0x00, 0x00, 0x1a, 0x26, 0x22, 0x1e, 0x02,
      0x1c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* a */
    { 0x00, 0x00, 0x3c, 0x22, 0x22, 0x22, 0x22,
      0x3c, 0x20, 0x20, 0x20, 0x00, 0x00 }, /* b */
    { 0x00, 0x00, 0x1c, 0x22, 0x20, 0x20, 0x22,
      0x1c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* c */
    { 0x00, 0x00, 0x1e, 0x22, 0x22, 0x22, 0x22,
      0x1e, 0x02, 0x02, 0x02, 0x00, 0x00 }, /* d */
    { 0x00, 0x00, 0x1c, 0x22, 0x20, 0x3e, 0x22,
      0x1c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* e */
    { 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x3c,
      0x10, 0x10, 0x12, 0x0c, 0x00, 0x00 }, /* f */
    { 0x1c, 0x22, 0x02, 0x1e, 0x22, 0x22, 0x22,
      0x1c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* g */
    { 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x32,
      0x2c, 0x20, 0x20, 0x20, 0x00, 0x00 }, /* h */
    { 0x00, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08,
      0x18, 0x00, 0x08, 0x00, 0x00, 0x00 }, /* i */
    { 0x18, 0x24, 0x24, 0x04, 0x04, 0x04, 0x04,
      0x0c, 0x00, 0x04, 0x00, 0x00, 0x00 }, /* j */
    { 0x00, 0x00, 0x22, 0x24, 0x28, 0x30, 0x28,
      0x24, 0x20, 0x20, 0x20, 0x00, 0x00 }, /* k */
    { 0x00, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08,
      0x08, 0x08, 0x08, 0x18, 0x00, 0x00 }, /* l */
    { 0x00, 0x00, 0x22, 0x2a, 0x2a, 0x2a, 0x2a,
      0x34, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* m */
    { 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x32,
      0x2c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* n */
    { 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x22,
      0x1c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* o */
    { 0x20, 0x20, 0x20, 0x3c, 0x22, 0x22, 0x22,
      0x3c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* p */
    { 0x02, 0x02, 0x02, 0x1e, 0x22, 0x22, 0x22,
      0x1e, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* q */
    { 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x32,
      0x2c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* r */
    { 0x00, 0x00, 0x1c, 0x22, 0x04, 0x18, 0x22,
      0x1c, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* s */
    { 0x00, 0x00, 0x0c, 0x12, 0x10, 0x10, 0x10,
      0x3c, 0x10, 0x10, 0x00, 0x00, 0x00 }, /* t */
    { 0x00, 0x00, 0x1a, 0x26, 0x22, 0x22, 0x22,
      0x22, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* u */
    { 0x00, 0x00, 0x08, 0x14, 0x14, 0x22, 0x22,
      0x22, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* v */
    { 0x00, 0x00, 0x14, 0x2a, 0x2a, 0x2a, 0x22,
      0x22, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* w */
    { 0x00, 0x00, 0x22, 0x14, 0x08, 0x08, 0x14,
      0x22, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* x */
    { 0x1c, 0x22, 0x02, 0x1a, 0x26, 0x22, 0x22,
      0x22, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* y */
    { 0x00, 0x00, 0x3e, 0x20, 0x10, 0x08, 0x04,
      0x3e, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* z */
    { 0x00, 0x06, 0x08, 0x08, 0x08, 0x08, 0x30,
      0x08, 0x08, 0x08, 0x08, 0x06, 0x00 }, /* { */
    { 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08,
      0x08, 0x08, 0x08, 0x08, 0x00, 0x00 }, /* | */
    { 0x00, 0x30, 0x08, 0x08, 0x08, 0x08, 0x06,
      0x08, 0x08, 0x08, 0x08, 0x30, 0x00 }, /* } */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x24, 0x2a, 0x12, 0x00, 0x00 }, /* ~ */
    { 0x3f, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
      0x21, 0x21, 0x21, 0x21, 0x21, 0x3f }, /*   */
};

/*---------------------------------------------------------------------------*/

#define MAXHST 128

static char history[MAXHST][MAXSTR];
static char command[MAXSTR];
static int  history_i = 0;
static int  history_j = 0;
static int  command_i = 0;

static int console_enable = 0;

static int console_w;
static int console_h;
static int console_x;
static int console_y;

static unsigned char console_r = 0x00;
static unsigned char console_g = 0xFF;
static unsigned char console_b = 0x00;

static unsigned char *console;

#define CONS_C(i, j) console[(console_w * i + j) * 4]
#define CONS_R(i, j) console[(console_w * i + j) * 4 + 1]
#define CONS_G(i, j) console[(console_w * i + j) * 4 + 2]
#define CONS_B(i, j) console[(console_w * i + j) * 4 + 3]

/*---------------------------------------------------------------------------*/

static void faded(const char *str)
{
    int i, l = strlen(str);

    for (i = 0; i < l; i++)
    {
        float k = (float) i / l;

        char buf[2] = { str[i], '\0' };

        color_console((1.0 - k), k, 0.0);
        print_console(buf);
    }
}

static void ident(void)
{
    faded("  |||  ELECTRO                           \n");
    faded("  O o  Copyright (C) 2005  Robert Kooima \n");
    faded("   -   http://www.evl.uic.edu/rlk/electro\n");

    console_enable = 0;
}

/*---------------------------------------------------------------------------*/

int set_console_enable(int b)
{
    console_enable = b;
    return 1;
}

int console_is_enabled(void)
{
    return console_enable;
}

/*---------------------------------------------------------------------------*/

int init_console(int w, int h)
{
    if ((console = (unsigned char *) calloc(w * h * 4, 1)))
    {
        console_w = w;
        console_h = h;
        console_x = 0;
        console_y = 0;

        ident();

        return 1;
    }
    return 0;
}

void draw_console(void)
{
    int i;
    int j;
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;

    if (console_enable)
    {
        glPushAttrib(GL_ENABLE_BIT);
        {
            /* Set the GL state for rendering the console. */

            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);

            /* Apply a pixel-for-pixel transformation. */

            glMatrixMode(GL_PROJECTION);
            {
                glLoadIdentity();
                glOrtho(0, get_window_w(),
                        0, get_window_h(), -1, +1);
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glLoadIdentity();
            }

            /* Draw the gray backdrop box. */

            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            glRecti(CONSOLE_X,
                    CONSOLE_Y,
                    CONSOLE_X + STEP_X * console_w,
                    CONSOLE_Y + STEP_Y * console_h);

            /* Draw the gray backdrop box. */

            glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
            glRecti(CONSOLE_X + STEP_X *  console_x,
                    CONSOLE_Y + STEP_Y *  console_y,
                    CONSOLE_X + STEP_X * (console_x + 1),
                    CONSOLE_Y + STEP_Y * (console_y + 1));

            /* Iterate over the character buffer rendering all glyphs. */

            for (i = 0; i < console_h; i++)
                for (j = 0; j < console_w; j++)
                    if (CONS_C(i, j))
                    {
                        if (CONS_R(i, j) != r ||
                            CONS_G(i, j) != g ||
                            CONS_B(i, j) != b)
                        {
                            r = CONS_R(i, j);
                            b = CONS_B(i, j);
                            g = CONS_G(i, j);
                            glColor3ub(r, g, b);
                        }

                        /* TODO: the glyphs are offset in the bytes. */

                        glRasterPos2i(CONSOLE_X + j * STEP_X - 2,
                                      CONSOLE_Y + i * STEP_Y);
                        glBitmap(8, 13, 0, 0, 0, 0, glyph[CONS_C(i, j) - 32]);
                    }
        }
        glPopAttrib();
    }
}

/*---------------------------------------------------------------------------*/

static void scroll_console(void)
{
    memmove(console + console_w * 4, console, console_w * (console_h - 1) * 4);
    memset(console, 0, console_w * 4);
}

static void clear_console_line(void)
{
    int i;

    /* Clear to the end of the line. */

    for (i = console_x; i < console_w; i++)
        CONS_C(console_y, i) = 0;
}

static void input_console_line(void)
{
    /* Scroll the console if necessary. */

    if (console_y == 0)
        scroll_console();
    else
        console_y = console_y - 1;
            
    /* Carriage return. */

    console_x = 0;
}

static void refresh_command(void)
{
    unsigned char r = console_r;
    unsigned char g = console_g;
    unsigned char b = console_b;

    color_console(1.0f, 1.0f, 1.0f);

    console_x = 0;

    print_console(command);
    clear_console_line();

    console_x = command_i;
    console_r = r;
    console_g = g;
    console_b = b;
}

int input_console(int symbol, int unicode)
{
    if (symbol == SDLK_RETURN)                      /* Execute the command. */

    {
        if (command_i > 0)
        {
            refresh_command();
            input_console_line();

            strncpy(history[history_i], command, MAXSTR);

            history_i = history_j = (history_i + 1) % MAXHST;
            command_i = 0;

            do_command(command);

            memset(command, 0, MAXSTR);
        }
    }

    else if (symbol == SDLK_LEFT || unicode == 2)   /* Cursor moves left. */
    {
        if (command_i > 0)
        {
            command_i--;
            console_x--;
        }
    }

    else if (symbol == SDLK_RIGHT || unicode == 6)  /* Cursor moves right. */
    {
        if (command[command_i])
        {
            command_i++;
            console_x++;
        }
    }

    else if (symbol == SDLK_HOME || unicode == 1)   /* Home */
        console_x = command_i = 0;

    else if (symbol == SDLK_END || unicode == 5)    /* End */
        console_x = command_i = strlen(command);

    else if (unicode == 11)                         /* Kill */
    {
        memset(command + command_i, 0, MAXSTR - command_i - 1);
        refresh_command();
    }

    else if (symbol == SDLK_UP)                     /* Previous history. */
    {
        history_j = (history_j - 1) % MAXHST;
        strncpy(command, history[history_j], MAXSTR);

        command_i = strlen(command);
        refresh_command();
    }

    else if (symbol == SDLK_DOWN)                   /* Next history. */
    {
        history_j = (history_j + 1) % MAXHST;
        strncpy(command, history[history_j], MAXSTR);

        command_i = strlen(command);
        refresh_command();
    }

    else if (symbol == SDLK_DELETE ||
             symbol == SDLK_BACKSPACE)              /* Delete a character. */
    {
        if (command_i > 0)
        {
            memmove(command + command_i - 1,
                    command + command_i, MAXSTR - command_i - 1);

            command_i--;
            refresh_command();
        }
    }
    
    else if (isprint(unicode))                      /* Insert a character. */
    {
        if (command_i < MAXSTR && console_x < console_w)
        {
            memmove(command + command_i + 1,
                    command + command_i, MAXSTR - command_i - 1);
            command[command_i] = (char) unicode;

            command_i++;
            refresh_command();
        }
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

void clear_console(void)
{
    memset(console, 0, console_w * console_h * 4);
}

void color_console(float r, float g, float b)
{
    console_r = (unsigned char) (r * 255.0);
    console_g = (unsigned char) (g * 255.0);
    console_b = (unsigned char) (b * 255.0);
}

void print_console(const char *str)
{
    int i, l = strlen(str);

    for (i = 0; i < l; i++)
    {
        if (str[i] == '\n')
        {
            if (console_y == 0)
                scroll_console();
            else
                console_y = console_y - 1;
            
            console_x = 0;
        }
        if (str[i] >= 32)
        {
            if (0 <= console_x && console_x < console_w &&
                0 <= console_y && console_y < console_h)
            {
                CONS_C(console_y, console_x) = str[i];
                CONS_R(console_y, console_x) = console_r;
                CONS_G(console_y, console_x) = console_g;
                CONS_B(console_y, console_x) = console_b;
            }
            console_x = console_x + 1;
        }
    }

    console_enable = 1;
}

void error_console(const char *str)
{
    unsigned char r = console_r;
    unsigned char g = console_g;
    unsigned char b = console_b;

    color_console(1.0f, 0.0f, 0.0f);

    print_console("Error: ");
    print_console(str);
    print_console("\n");

    console_r = r;
    console_g = g;
    console_b = b;
}

/*---------------------------------------------------------------------------*/