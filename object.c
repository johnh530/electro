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

#include <stdio.h>
#include <string.h>

#include "opengl.h"
#include "object.h"

#define MAXSTR 256

/*---------------------------------------------------------------------------*/
/* Vector caches                                                             */

#define MAXV 32768

static float vv[MAXV][3];
static float nv[MAXV][3];
static float tv[MAXV][2];
static int   iv[MAXV][4];

static int   vc;
static int   nc;
static int   tc;
static int   ic;

/*---------------------------------------------------------------------------*/
/* Object element cache                                                      */

#define MAXMTRL    64
#define MAXVERT 32768
#define MAXFACE 16384
#define MAXSURF   128

static char               namev[MAXMTRL][MAXSTR];
static struct object_mtrl mtrlv[MAXMTRL];
static struct object_vert vertv[MAXVERT];
static struct object_face facev[MAXFACE];
static struct object_surf surfv[MAXSURF];

static int mtrlc;
static int vertc;
static int facec;
static int surfc;

/*---------------------------------------------------------------------------*/

static void *memdup(void *src, size_t num, size_t len)
{
    void *dst;

    if ((dst = malloc(num * len)))
        memcpy(dst, src, num * len);

    return dst;
}

/*---------------------------------------------------------------------------*/

static void read_newmtl(const char *name)
{
    if (mtrlc < MAXMTRL)
    {
        mtrlc++;

        strncpy(namev[mtrlc], name, MAXSTR);

        /* Default diffuse */

        mtrlv[mtrlc].d[0] = 1.0f;
        mtrlv[mtrlc].d[1] = 1.0f;
        mtrlv[mtrlc].d[2] = 1.0f;
        mtrlv[mtrlc].d[3] = 1.0f;

        /* Default ambient */

        mtrlv[mtrlc].a[0] = 0.2f;
        mtrlv[mtrlc].a[1] = 0.2f;
        mtrlv[mtrlc].a[2] = 0.2f;

        /* Default specular */

        mtrlv[mtrlc].s[0] = 0.2f;
        mtrlv[mtrlc].s[1] = 0.2f;
        mtrlv[mtrlc].s[2] = 0.2f;

        /* Default emmisive */

        mtrlv[mtrlc].e[0] = 0.0f;
        mtrlv[mtrlc].e[1] = 0.0f;
        mtrlv[mtrlc].e[2] = 0.0f;

        /* Default shininess */

        mtrlv[mtrlc].x[0] = 0.0f;
    }
}

static void read_map_Kd(const char *line)
{
}

static void read_Kd(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f %f %f", mtrlv[mtrlc].d + 0,
                                 mtrlv[mtrlc].d + 1,
                                 mtrlv[mtrlc].d + 2);
}

static void read_Ka(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f %f %f", mtrlv[mtrlc].a + 0,
                                 mtrlv[mtrlc].a + 1,
                                 mtrlv[mtrlc].a + 2);
}

static void read_Ks(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f %f %f", mtrlv[mtrlc].s + 0,
                                 mtrlv[mtrlc].s + 1,
                                 mtrlv[mtrlc].s + 2);
}

static void read_Ke(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f %f %f", mtrlv[mtrlc].e + 0,
                                 mtrlv[mtrlc].e + 1,
                                 mtrlv[mtrlc].e + 2);
}

static void read_Ns(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f", mtrlv[mtrlc].d + 3);
}

static void read_d(const char *line)
{
    if (mtrlc >= 0)
        sscanf(line, "%f", mtrlv[mtrlc].x + 0);
}

static void read_mtl(const char *filename)
{
    char line[MAXSTR];
    FILE *fin;

    mtrlc = 0;

    if ((fin = fopen(filename, "r")))
    {
        /* Process each line, invoking the handler for each keyword. */

        while (fgets(line, MAXSTR, fin))
        {
            if      (strncmp(line, "newmtl", 6) == 0) read_newmtl(line + 7);
            else if (strncmp(line, "map_Kd", 6) == 0) read_map_Kd(line + 7);

            else if (strncmp(line, "Kd", 2) == 0) read_Kd(line + 2);
            else if (strncmp(line, "Ka", 2) == 0) read_Ka(line + 2);
            else if (strncmp(line, "Ks", 2) == 0) read_Ks(line + 2);
            else if (strncmp(line, "Ke", 2) == 0) read_Ke(line + 2);
            else if (strncmp(line, "Ns", 2) == 0) read_Ns(line + 2);
            else if (strncmp(line, "d",  1) == 0) read_d (line + 2);
        }
        fclose(fin);
    }
}

static int find_mtl(const char *name)
{
    int i;

    for (i = 0; i < mtrlc; ++i)
        if (strncmp(namev[i], name, MAXSTR) == 0)
            return i;

    return 0;
}

/*---------------------------------------------------------------------------*/

static void read_f(const char *line)
{
    const char *c = line;
    int dc, i, i0 = ic;

    /* Scan down the face string recording index set specifications. */

    while (ic < MAXV && sscanf(c, "%d/%d/%d%n", &iv[ic][0],
                                                &iv[ic][1],
                                                &iv[ic][2], &dc) >= 3)
    {
        iv[ic][3] = ic;

        /* If we've seen this index set before, note the associated vertex. */

        for (i = 0; i < ic; ++i)
            if (iv[ic][0] == iv[i][0] &&
                iv[ic][1] == iv[i][1] &&
                iv[ic][2] == iv[i][2])
            {
                iv[ic][3]  = iv[i][3];
                break;
            }

        /* If we haven't seen this index set, create a new vertex. */

        if (i == ic)
        {
            /* Copy vector data from the cache. */

            vertv[vertc].v[0] = vv[iv[i][0]][0];
            vertv[vertc].v[1] = vv[iv[i][0]][1];
            vertv[vertc].v[2] = vv[iv[i][0]][2];

            vertv[vertc].n[0] = nv[iv[i][1]][0];
            vertv[vertc].n[1] = nv[iv[i][1]][1];
            vertv[vertc].n[2] = nv[iv[i][1]][2];

            vertv[vertc].t[0] = tv[iv[i][2]][0];
            vertv[vertc].t[1] = tv[iv[i][2]][1];

            /* Associate the index set with the new vertex. */

            iv[ic++][3] = vertc++;
        }

        c += dc;
    }

    /* Convert our N new index sets into N-2 new triangles. */

    for (i = i0; i < ic - 2 && facec && MAXFACE; ++i)
    {
        facev[facec].vi[0] = iv[i0][3];
        facev[facec].vi[1] = iv[i + 1][3];
        facev[facec].vi[2] = iv[i + 2][3];

        facec++;
    }
}

/*---------------------------------------------------------------------------*/

static void read_g(void)
{
    if (surfc >= 0)
    {
        /* Close out the existing surface by copying the face cache. */

        surfv[surfc].fv =
            (struct object_face *) memdup(facev,
                                          facec, sizeof (struct object_face));
        facec = 0;
    }

    if (surfc < MAXSURF)
    {
        /* Initialize a new empty surface. */

        surfc++;

        surfv[surfc].mi =    0;
        surfv[surfc].fc =    0;
        surfv[surfc].fv = NULL;

        /* Reset all the vector caches. */

        vc = 0;
        tc = 0;
        nc = 0;
        ic = 0;
    }
}

static void read_mtllib(const char *line)
{
    read_mtl(line);
    read_g();
}

static void read_usemtl(const char *line)
{
    if (surfc >= 0)
        surfv[surfc].mi = find_mtl(line);
}

static void read_vt(const char *line)
{
    if (tc < MAXV && sscanf(line, "%f %f", &tv[tc][0],
                                           &tv[tc][1]) == 2) tc++;
}

static void read_vn(const char *line)
{
    if (nc < MAXV && sscanf(line, "%f %f %f", &nv[nc][0],
                                              &nv[nc][1],
                                              &nv[nc][2]) == 3) nc++;
}

static void read_v(const char *line)
{
    if (vc < MAXV && sscanf(line, "%f %f %f", &vv[vc][0],
                                              &vv[vc][1],
                                              &vv[vc][2]) == 3) vc++;
}

/*---------------------------------------------------------------------------*/

static void read_obj(const char *filename, struct object *O)
{
    char line[MAXSTR];
    FILE *fin;

    /* Initialize the element caches. */

    mtrlc = -1;
    vertc =  0;
    facec =  0;
    surfc = -1;

    if ((fin = fopen(filename, "r")))
    {
        /* Process each line, invoking the handler for each keyword. */

        while (fgets(line, MAXSTR, fin))
        {
            if      (strncmp(line, "mtllib", 6) == 0) read_mtllib(line + 7);
            else if (strncmp(line, "usemtl", 6) == 0) read_usemtl(line + 7);

            else if (strncmp(line, "g",  1) == 0) read_g ();
            else if (strncmp(line, "f",  1) == 0) read_f (line + 1);
            else if (strncmp(line, "vt", 2) == 0) read_vt(line + 2);
            else if (strncmp(line, "vn", 2) == 0) read_vn(line + 2);
            else if (strncmp(line, "v",  1) == 0) read_v (line + 1);
        }
        fclose(fin);
    }

    /* Close out the object by copying the element caches. */

    O->mv = (struct object_mtrl *) memdup(mtrlv,
                                          mtrlc, sizeof (struct object_mtrl));
    O->vv = (struct object_vert *) memdup(vertv,
                                          vertc, sizeof (struct object_vert));
    O->sv = (struct object_surf *) memdup(surfv,
                                          surfc, sizeof (struct object_surf));

    O->mc = mtrlc;
    O->vc = vertc;
    O->sc = surfc;
}

/*---------------------------------------------------------------------------*/
