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
#include <stdlib.h>
#include <math.h>

#include "opengl.h"
#include "vector.h"
#include "buffer.h"
#include "camera.h"
#include "utility.h"
#include "image.h"
#include "event.h"
#include "brush.h"

/*---------------------------------------------------------------------------*/

#define MAX_PARAM 96

struct brush
{
    int  count;
    int  state;
    int  flags;
    int  image[4];

    char *file;
    char *name;
    char *frag;
    char *vert;

    float d[4];
    float s[4];
    float a[4];
    float x[1];

    GLuint  frag_prog;
    GLuint  vert_prog;

    float  frag_param[MAX_PARAM][4];
    float  vert_param[MAX_PARAM][4];
};

static vector_t brush;

/*---------------------------------------------------------------------------*/

static struct brush *get_brush(int i)
{
    return (struct brush *) vecget(brush, i);
}

static int new_brush(void)
{
    int i, n = vecnum(brush);

    for (i = 0; i < n; ++i)
        if (get_brush(i)->count == 0)
            return i;

    return vecadd(brush);
}

/*===========================================================================*/

static const char *parse_name(const char *line)
{
    static char name[MAXSTR];
    char *last;

    /* Extract a name parameter from the given MTL line. */

    if ((last = strrchr(line, ' ')))
    {
        sscanf(last, "%s", name);
        return name;
    }
    return NULL;
}

static void load_brush(struct brush *b, const char *file, const char *name)
{
    char L[MAXSTR];
    char W[MAXSTR];

    FILE *fin;

    int scanning = 1;
    int n        = 0;

    if ((fin = open_file(file, "r")))
    {
        /* Process each line of the MTL file. */

        while (fgets(L, MAXSTR, fin))
        {
            /* Extract the keyword from the current line. */

            if (sscanf(L, "%s%n", W, &n) >= 1)
            {
                if (scanning)
                {
                    /* Determine if we've found the MTL we're looking for. */

                    if (!strcmp(W, "newmtl"))
                        scanning = strcmp(parse_name(L + n), name);
                }
                else
                {
                    /* If we found our material, parse until the next one. */

                    if (!strcmp(W, "newmtl"))
                        return;

                    /* Parse this material's properties. */

                    else if (strcmp(W, "map_Kd") == 0)
                    {
                        b->image[0] = send_create_image(parse_name(L + n),
                                                        0, 0, 0, 0, 0);
                    }
                    else if (strcmp(W, "Kd") == 0)
                    {
                        sscanf(L + n, "%f %f %f", b->d, b->d + 1, b->d + 2);
                        b->flags |= BRUSH_DIFFUSE;
                    }
                    else if (strcmp(W, "Ka") == 0)
                    {
                        sscanf(L + n, "%f %f %f", b->a, b->a + 1, b->a + 2);
                        b->flags |= BRUSH_AMBIENT;
                    }
                    else if (strcmp(W, "Ks") == 0)
                    {
                        sscanf(L + n, "%f %f %f", b->s, b->s + 1, b->s + 2);
                        b->flags |= BRUSH_SPECULAR;
                    }
                    else if (strcmp(W, "Ns") == 0)
                    {
                        sscanf(L + n, "%f", b->x);
                        b->flags |= BRUSH_SHINY;
                    }
                    else if (strcmp(W, "d") == 0)
                    {
                        sscanf(L + n, "%f", b->d + 3);
                        b->flags |= (b->d[3] < 1) ? BRUSH_TRANSPARENT : 0;
                    }
                }
            }
        }
        fclose(fin);
    }
    else error("MTL file '%s': %s", file, system_error());
}

/*---------------------------------------------------------------------------*/

int dupe_create_brush(int i)
{
    get_brush(i)->count++;
    return i;
}

int send_create_brush(const char *file, const char *name)
{
    int i, n = vecnum(brush);

    /* Scan the current brushes for an existing instance. */

    for (i = 0; i < n; ++i)
    {
        struct brush *b = get_brush(i);

        if (file && b->file && strcmp(b->file, file) == 0 &&
            name && b->name && strcmp(b->name, name) == 0)
            return dupe_create_brush(i);
    }

    /* Didn't find it.  It's new. */

    if ((i = new_brush()) >= 0)
    {
        struct brush *b = get_brush(i);

        /* Note the file and material names. */

        if (file) b->file = memdup(file, strlen(file) + 1, 1);
        if (name) b->name = memdup(name, strlen(name) + 1, 1);

        /* Set some default values. */

        b->count = 1;
        b->state = 0;
        b->flags = BRUSH_DIFFUSE | BRUSH_SPECULAR |
                   BRUSH_AMBIENT | BRUSH_SHINY;

        b->d[0] = BRUSH_DIFFUSE_R;
        b->d[1] = BRUSH_DIFFUSE_G;
        b->d[2] = BRUSH_DIFFUSE_B;
        b->d[3] = BRUSH_DIFFUSE_A;

        b->s[0] = BRUSH_SPECULAR_R;
        b->s[1] = BRUSH_SPECULAR_G;
        b->s[2] = BRUSH_SPECULAR_B;
        b->s[3] = BRUSH_SPECULAR_A;

        b->a[0] = BRUSH_AMBIENT_R;
        b->a[1] = BRUSH_AMBIENT_G;
        b->a[2] = BRUSH_AMBIENT_B;
        b->a[3] = BRUSH_AMBIENT_A;

        b->x[0] = BRUSH_SHININESS;

        /* Load and pack the brush. */

        if (file && name)
            load_brush(b, file, name);

        send_event(EVENT_CREATE_BRUSH);
        send_index(b->flags);

        send_array(b->image, 4, sizeof (int));
        send_array(b->d,     4, sizeof (float));
        send_array(b->s,     4, sizeof (float));
        send_array(b->a,     4, sizeof (float));
        send_array(b->x,     1, sizeof (float));

        return i;
    }
    return -1;
}

void recv_create_brush(void)
{
    struct brush *b = get_brush(new_brush());

    b->flags = recv_index();
    b->count = 1;

    recv_array(b->image, 4, sizeof (int));
    recv_array(b->d,     4, sizeof (float));
    recv_array(b->s,     4, sizeof (float));
    recv_array(b->a,     4, sizeof (float));
    recv_array(b->x,     1, sizeof (float));
}

/*---------------------------------------------------------------------------*/

static int free_brush(int i)
{
    struct brush *b = get_brush(i);

    if (i > 0 && --b->count <= 0)
    {
        fini_brush(i);

        if (b->file) free(b->file);
        if (b->name) free(b->name);
        if (b->frag) free(b->frag);
        if (b->vert) free(b->vert);

        if (b->image[0]) send_delete_image(b->image[0]);
        if (b->image[1]) send_delete_image(b->image[1]);
        if (b->image[2]) send_delete_image(b->image[2]);
        if (b->image[3]) send_delete_image(b->image[3]);

        memset(b, 0, sizeof (struct brush));

        return 1;
    }
    return 0;
}

void send_delete_brush(int i)
{
    if (free_brush(i))
    {
        send_event(EVENT_DELETE_BRUSH);
        send_index(i);
    }
}

void recv_delete_brush(void)
{
    free_brush(recv_index());
}

/*---------------------------------------------------------------------------*/

void send_set_brush_image(int i, int j, int k)
{
    struct brush *b = get_brush(i);

    dupe_create_image(j);
    send_delete_image(b->image[k]);

    send_event(EVENT_SET_BRUSH_IMAGE);
    send_index(i);
    send_index((b->image[k] = j));
    send_index(k);
}

void recv_set_brush_image(void)
{
    int i = recv_index();
    int j = recv_index();
    int k = recv_index();

    get_brush(i)->image[k] = j;
}

/*---------------------------------------------------------------------------*/

void send_set_brush_flags(int i, int flags, int state)
{
    struct brush *b = get_brush(i);

    send_event(EVENT_SET_BRUSH_FLAGS);
    send_index(i);
    send_index(flags);
    send_index(state);

    if (state)
        b->flags = b->flags | ( flags);
    else
        b->flags = b->flags & (~flags);
}

void recv_set_brush_flags(void)
{
    int i     = recv_index();
    int flags = recv_index();
    int state = recv_index();

    struct brush *b = get_brush(i);

    if (state)
        b->flags = b->flags | ( flags);
    else
        b->flags = b->flags & (~flags);
}

/*---------------------------------------------------------------------------*/

void send_set_brush_frag_prog(int i, const char *text)
{
    int n = text ? (strlen(text) + 1) : 0;

    struct brush *b = get_brush(i);

    send_event(EVENT_SET_BRUSH_FRAG_PROG);
    send_index(i);
    send_index(n);

    fini_brush(i);

    /* Free the old fragment program buffer. */

    if (b->frag)
    {
        free(b->frag);
        b->frag = NULL;
    }

    /* Copy the new fragment program buffer. */

    if (n > 0)
    {
        b->frag = memdup(text, n, 1);
        send_array(text, n, 1);
    }
}

void recv_set_brush_frag_prog(void)
{
    int i = recv_index();
    int n = recv_index();

    struct brush *b = get_brush(i);

    fini_brush(i);

    /* Free the old fragment program buffer. */

    if (b->frag)
    {
        free(b->frag);
        b->frag = NULL;
    }

    /* Receive the new fragment program buffer. */

    if (n > 0)
    {
        b->frag = (char *) malloc(n);
        recv_array(b->frag, n, 1);
    }
}

/*---------------------------------------------------------------------------*/

void send_set_brush_vert_prog(int i, const char *text)
{
    int n = text ? (strlen(text) + 1) : 0;

    struct brush *b = get_brush(i);

    send_event(EVENT_SET_BRUSH_VERT_PROG);
    send_index(i);
    send_index(n);

    fini_brush(i);

    /* Free the old vertex program buffer. */

    if (b->vert)
    {
        free(b->vert);
        b->vert = NULL;
    }

    /* Copy the new vertex program buffer. */

    if (n > 0)
    {
        b->vert = memdup(text, n, 1);
        send_array(text, n, 1);
    }
}

void recv_set_brush_vert_prog(void)
{
    int i = recv_index();
    int n = recv_index();

    struct brush *b = get_brush(i);

    fini_brush(i);

    /* Free the old vertex program buffer. */

    if (b->vert)
    {
        free(b->vert);
        b->vert = NULL;
    }

    /* Receive the new vertex program buffer. */

    if (n > 0)
    {
        b->vert = (char *) malloc(n);
        recv_array(b->vert, n, 1);
    }
}

/*---------------------------------------------------------------------------*/

void send_set_brush_frag_param(int i, int p, const float v[4])
{
    struct brush *b = get_brush(i);

    b->frag_param[p][0] = v[0];
    b->frag_param[p][1] = v[1];
    b->frag_param[p][2] = v[2];
    b->frag_param[p][3] = v[3];

    send_event(EVENT_SET_BRUSH_FRAG_PARAM);
    send_index(i);
    send_index(p);
    send_array(b->frag_param[p], 4, sizeof (float));

    if (b->frag_prog && GL_has_fragment_program)
    {
        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, b->frag_prog);
        glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,
                                      p, b->frag_param[p]);
    }
}

void recv_set_brush_frag_param(void)
{
    struct brush *b = get_brush(recv_index());
    int           p = recv_index();

    recv_array(b->frag_param[p], 4, sizeof (float));

    if (b->frag_prog && GL_has_fragment_program)
    {
        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, b->frag_prog);
        glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,
                                      p, b->frag_param[p]);
    }
}

/*---------------------------------------------------------------------------*/

void send_set_brush_vert_param(int i, int p, const float v[4])
{
    struct brush *b = get_brush(i);

    b->vert_param[p][0] = v[0];
    b->vert_param[p][1] = v[1];
    b->vert_param[p][2] = v[2];
    b->vert_param[p][3] = v[3];

    send_event(EVENT_SET_BRUSH_VERT_PARAM);
    send_index(i);
    send_index(p);
    send_array(b->vert_param[p], 4, sizeof (float));

    if (b->vert_prog && GL_has_vertex_program)
    {
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, b->vert_prog);
        glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB,
                                      p, b->vert_param[p]);
    }
}

void recv_set_brush_vert_param(void)
{
    struct brush *b = get_brush(recv_index());
    int           p = recv_index();

    recv_array(b->vert_param[p], 4, sizeof (float));

    if (b->vert_prog && GL_has_vertex_program)
    {
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, b->vert_prog);
        glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB,
                                      p, b->vert_param[p]);
    }
}

/*---------------------------------------------------------------------------*/

void send_set_brush_color(int i, const float d[4],
                                 const float s[4],
                                 const float a[4],
                                 const float x[1], int f)
{
    struct brush *b = get_brush(i);

    b->flags |= f;

    if (f & BRUSH_DIFFUSE)
    {
        b->d[0] = d[0];
        b->d[1] = d[1];
        b->d[2] = d[2];
        b->d[3] = d[3];
    }
    if (f & BRUSH_SPECULAR)
    {
        b->s[0] = s[0];
        b->s[1] = s[1];
        b->s[2] = s[2];
        b->s[3] = s[3];
    }
    if (f & BRUSH_AMBIENT)
    {
        b->a[0] = a[0];
        b->a[1] = a[1];
        b->a[2] = a[2];
        b->a[3] = a[3];
    }
    if (f & BRUSH_SHINY)
        b->x[0] = x[0];

    send_event(EVENT_SET_BRUSH_COLOR);
    send_index(i);
    send_array(b->d, 4, sizeof (float));
    send_array(b->s, 4, sizeof (float));
    send_array(b->a, 4, sizeof (float));
    send_array(b->x, 1, sizeof (float));
    send_index(b->flags);
}

void recv_set_brush_color(void)
{
    int i = recv_index();

    struct brush *b = get_brush(i);

    recv_array(b->d, 4, sizeof (float));
    recv_array(b->s, 4, sizeof (float));
    recv_array(b->a, 4, sizeof (float));
    recv_array(b->x, 1, sizeof (float));

    b->flags = recv_index();
}

/*===========================================================================*/

void init_brush(int i)
{
    struct brush *b = get_brush(i);

    if (b->state == 0)
    {
        int p;

        /* Initialize any vertex program and parameters. */

        if (b->vert && GL_has_vertex_program)
        {
            b->vert_prog = opengl_vert_prog(b->vert);

            for (p = 0; p < MAX_PARAM; ++p)
                glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB,
                                              p, b->vert_param[p]);
        }

        /* Initialize any fragment program and parameters. */

        if (b->frag && GL_has_fragment_program)
        {
            b->frag_prog = opengl_frag_prog(b->frag);

            for (p = 0; p < MAX_PARAM; ++p)
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,
                                              p, b->frag_param[p]);
        }

        b->state = 1;
    }
}

void fini_brush(int i)
{
    struct brush *b = get_brush(i);

    if (b->state == 1)
    {
        /* Finalize any vertex and fragment programs. */

        if (GL_has_vertex_program)
            if (glIsProgramARB(b->vert_prog))
                glDeleteProgramsARB(1, &b->vert_prog);

        if (GL_has_fragment_program)
            if (glIsProgramARB(b->frag_prog))
                glDeleteProgramsARB(1, &b->frag_prog);
                
        b->vert_prog = 0;
        b->frag_prog = 0;
        b->state     = 0;
    }
}

/*---------------------------------------------------------------------------*/

static void set_env_map(void)
{
    float M[16];

    get_camera_rot(M);

    /* Set the given texture to generate cube map texture coordinates. */

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(M);
    glMatrixMode(GL_MODELVIEW);
}

static void set_sky_map(void)
{
    float S[4] = { 1, 0, 0, 0 };
    float T[4] = { 0, 1, 0, 0 };
    float R[4] = { 0, 0, 1, 0 };

    /* Set the given texture to generate sky map texture coordinates. */

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    glTexGenfv(GL_S, GL_OBJECT_PLANE, S);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, T);
    glTexGenfv(GL_R, GL_OBJECT_PLANE, R);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

static void set_tex_map(void)
{
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

static void set_tex_gen(GLenum texture, int env, int sky)
{
    if (GL_has_multitexture)
    {
        glActiveTextureARB(texture);

        if      (env) set_env_map();
        else if (sky) set_sky_map();
        else          set_tex_map();

        glActiveTextureARB(GL_TEXTURE0_ARB);
    }
}

static void apply_image(GLenum texture, int image)
{
    if (GL_has_multitexture)
    {
        glActiveTextureARB(texture);

        /* Always apply texture 0, but disable all other unused textures. */

        if (texture == GL_TEXTURE0_ARB || image != 0)
            draw_image(image);
        else
            glDisable(GL_TEXTURE_2D);
    }
    else draw_image(image);
}

/*---------------------------------------------------------------------------*/

int draw_brush(int i, float a)
{
    float d[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    struct brush *b = get_brush(i);
    
    if (b->count)
    {
        init_brush(i);

        /* Modulate the diffuse color by the current value. */

        d[0] = b->d[0];
        d[1] = b->d[1];
        d[2] = b->d[2];
        d[3] = b->d[3] * a;

        /* Apply the textures. */

        apply_image(GL_TEXTURE3_ARB, b->image[3]);
        apply_image(GL_TEXTURE2_ARB, b->image[2]);
        apply_image(GL_TEXTURE1_ARB, b->image[1]);
        apply_image(GL_TEXTURE0_ARB, b->image[0]);

        /* Enable texture coordinate generation, if requested. */

        set_tex_gen(GL_TEXTURE3_ARB, (b->flags & BRUSH_ENV_MAP_3),
                                     (b->flags & BRUSH_SKY_MAP_3));
        set_tex_gen(GL_TEXTURE2_ARB, (b->flags & BRUSH_ENV_MAP_2),
                                     (b->flags & BRUSH_SKY_MAP_2));
        set_tex_gen(GL_TEXTURE1_ARB, (b->flags & BRUSH_ENV_MAP_1),
                                     (b->flags & BRUSH_SKY_MAP_1));
        set_tex_gen(GL_TEXTURE0_ARB, (b->flags & BRUSH_ENV_MAP_0),
                                     (b->flags & BRUSH_SKY_MAP_0));

        /* Enable vertex and fragment programs, if specified. */

        if (b->frag_prog && GL_has_fragment_program)
        {
            glEnable(GL_FRAGMENT_PROGRAM_ARB);
            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, b->frag_prog);
        }
        if (b->vert_prog && GL_has_vertex_program)
        {
            glEnable(GL_VERTEX_PROGRAM_ARB);
            glBindProgramARB(GL_VERTEX_PROGRAM_ARB,   b->vert_prog);
        }

        /* Disable lighting, if requested. */

        if (b->flags & BRUSH_UNLIT)
        {
            glDisable(GL_LIGHTING);
            glColor4fv(d);
        }
        else
        {
            /* Apply the material properties. */
    
            if (b->flags & BRUSH_DIFFUSE)
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   d);
            if (b->flags & BRUSH_AMBIENT)
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   b->a);
            if (b->flags & BRUSH_SPECULAR)
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  b->s);
            if (b->flags & BRUSH_SHINY)
                glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, b->x);
        }
    }

    /* Return an indication that this brush is transparent. */

    return (d[3] < 1) | (b->flags & BRUSH_TRANSPARENT);
}

/*---------------------------------------------------------------------------*/

int get_brush_w(int i)
{
    return get_image_w(get_brush(i)->image[0]);
}

int get_brush_h(int i)
{
    return get_image_h(get_brush(i)->image[0]);
}

int get_brush_t(int i)
{
    return ((get_brush(i)->flags & BRUSH_TRANSPARENT) ||
            (get_brush(i)->d[3]  < 1.0));
}

/*---------------------------------------------------------------------------*/

void free_brushes(void)
{
    int i, n = vecnum(brush);

    for (i = 1; i < n; ++i)
        while (get_brush(i)->count)
            free_brush(i);
}

void init_brushes(void)
{
    int i, n = vecnum(brush);

    for (i = 0; i < n; ++i)
        if (get_brush(i)->count)
            init_brush(i);
}

void fini_brushes(void)
{
    int i, n = vecnum(brush);

    for (i = 0; i < n; ++i)
        if (get_brush(i)->count)
            fini_brush(i);
}

/*===========================================================================*/

int startup_brush(void)
{
    if ((brush = vecnew(16, sizeof (struct brush))))
    {
        int i;

        /* Create a default brush. */

        if ((i = new_brush()) >= 0)
        {
            struct brush *b = get_brush(i);

            b->count = 1;
            b->state = 0;
            b->flags = BRUSH_DIFFUSE | BRUSH_SPECULAR |
                       BRUSH_AMBIENT | BRUSH_SHINY;

            b->d[0] = BRUSH_DIFFUSE_R;
            b->d[1] = BRUSH_DIFFUSE_G;
            b->d[2] = BRUSH_DIFFUSE_B;
            b->d[3] = BRUSH_DIFFUSE_A;

            b->s[0] = BRUSH_SPECULAR_R;
            b->s[1] = BRUSH_SPECULAR_G;
            b->s[2] = BRUSH_SPECULAR_B;
            b->s[3] = BRUSH_SPECULAR_A;

            b->a[0] = BRUSH_AMBIENT_R;
            b->a[1] = BRUSH_AMBIENT_G;
            b->a[2] = BRUSH_AMBIENT_B;
            b->a[3] = BRUSH_AMBIENT_A;

            b->x[0] = BRUSH_SHININESS;

            return 1;
        }
    }
    return 0;
}
