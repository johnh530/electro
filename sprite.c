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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>

#include "opengl.h"
#include "shared.h"
#include "server.h"
#include "entity.h"
#include "sprite.h"

/*---------------------------------------------------------------------------*/

static struct sprite *S     = NULL;
static int            S_max =   64;

#define sprite_exists(id) (S && 0 <= id && id < S_max && S[id].texture)

/*---------------------------------------------------------------------------*/

static int unused_sprite(void)
{
    int id = 0;

    if (S)
        for (id = 1; id < S_max; ++id)
            if (S[id].texture == 0)
                break;

    return id;
}

static int expand_sprite(void)
{
    struct sprite *T;

    if (S == NULL)
    {
        if ((S = (struct sprite *) calloc(S_max, sizeof (struct sprite))))
            return 1;
    }

    if ((T = (struct sprite *) realloc(S, S_max * 2 * sizeof (struct sprite))))
    {
        memset(T + S_max, 0, S_max * sizeof (struct sprite));
        S_max *= 2;
        S      = T;

        return 1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

int sprite_create(const char *filename)
{
    int id;

    /* If the existing buffer still has room... */

    if ((id = unused_sprite()))
    {
        /* Initialize the new sprite. */

        if (mpi_isroot())
            server_send(EVENT_SPRITE_CREATE);

        /* Syncronize the new sprite. */

        mpi_share_integer(1, &id);

        S[id].texture = shared_load_texture(filename, &S[id].w, &S[id].h);
        S[id].a       = 1.0f;

        /* Encapsulate this new sprite in an entity. */

        return entity_create(TYPE_SPRITE, id);
    }

    /* If the buffer is full, double its size.  Retry. */

    if (expand_sprite())
        return sprite_create(filename);

    /* If the buffer cannot be doubled, fail. */

    return -1;
}

void sprite_render(int id)
{
    if (sprite_exists(id))
    {
        glBindTexture(GL_TEXTURE_2D, S[id].texture);
                
        glBegin(GL_QUADS);
        {
            int dx = S[id].w / 2;
            int dy = S[id].h / 2;

            glTexCoord2i(0, 0); glVertex2f(-dx, +dy);
            glTexCoord2i(0, 1); glVertex2f(-dx, -dy);
            glTexCoord2i(1, 1); glVertex2f(+dx, -dy);
            glTexCoord2i(1, 0); glVertex2f(+dx, +dy);
        }
        glEnd();
    }
}

/*---------------------------------------------------------------------------*/

/* This function should be called only by the entity delete function. */

void sprite_delete(int id)
{
    /* Assume delete was triggered by entity delete.  Share the descriptor. */

    mpi_share_integer(1, &id);

    /* Release the sprite object. */

    if (glIsTexture(S[id].texture))
        glDeleteTextures(1, &S[id].texture);

    memset(S + id, 0, sizeof (struct sprite));
}

/*---------------------------------------------------------------------------*/
