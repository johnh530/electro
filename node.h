/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    TOTAL PERSPECTIVE VORTEX is free software;  you can redistribute it    */
/*    and/or modify it under the terms of the  GNU General Public License    */
/*    as published by the  Free Software Foundation;  either version 2 of    */
/*    the License, or (at your option) any later version.                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef NODE_H
#define NODE_H

/*---------------------------------------------------------------------------*/

struct node
{
    int seed;
    int type;

    int star_first;
    int star_count;

    float position[3];
    float radius;
};

/*---------------------------------------------------------------------------*/

int  node_init(int);
void node_draw(int, int);

/*---------------------------------------------------------------------------*/

#endif
