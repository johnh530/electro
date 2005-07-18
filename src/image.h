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

#ifndef IMAGE_H
#define IMAGE_H

/*---------------------------------------------------------------------------*/

int startup_image(void);

/*---------------------------------------------------------------------------*/

GLuint make_texture(const void *, int, int, int);

/*---------------------------------------------------------------------------*/

int  send_create_image(const char *);
void recv_create_image(void);

/*---------------------------------------------------------------------------*/

void get_image_p(int, int, int, unsigned char[4]);
int  get_image_w(int);
int  get_image_h(int);

/*---------------------------------------------------------------------------*/

void  init_image(int);
void  fini_image(int);
void  draw_image(int);
void  dupe_image(int);
void  free_image(int);

void  init_images(void);
void  fini_images(void);

/*---------------------------------------------------------------------------*/

#endif
