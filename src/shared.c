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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef MPI
#include <mpi.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif

#include "image.h"
#include "shared.h"
#include "camera.h"

/*---------------------------------------------------------------------------*/
/* Optional MPI function abstractions                                        */

int mpi_assert(int err)
{
#ifdef MPI
    char buf[256];
    int len = 256;

    if (err == MPI_SUCCESS)
        return 1;
    else
    {
        MPI_Error_string(err, buf, &len);
        fprintf(stderr, "MPI Error: %s\n", buf);
        return 0;
    }
#else
    return 1;
#endif
}

int mpi_rank(void)
{
    int rank = 0;
#ifdef MPI
    mpi_assert(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
#endif
    return rank;
}

int mpi_size(void)
{
    int size = 0;
#ifdef MPI
    mpi_assert(MPI_Comm_size(MPI_COMM_WORLD, &size));
#endif
    return size;
}

int mpi_isroot(void)
{
    return (mpi_rank() == 0);
}

void mpi_barrier(void)
{
#ifdef MPI
#ifndef NDEBUG
    printf("%d of %d: barrier\n", mpi_rank(), mpi_size());
#endif
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}

/*---------------------------------------------------------------------------*/
/* Optional MPI broadcast function abstractions                              */

int mpi_share_byte(int bc, void *bv)
{
#ifdef MPI
    return mpi_assert(MPI_Bcast(bv, bc, MPI_BYTE, 0, MPI_COMM_WORLD));
#else
    return 1;
#endif
}

int mpi_share_char(int cc, char *cv)
{
#ifdef MPI
    return mpi_assert(MPI_Bcast(cv, cc, MPI_CHAR, 0, MPI_COMM_WORLD));
#else
    return 1;
#endif
}

int mpi_share_float(int fc, float *fv)
{
#ifdef MPI
    return mpi_assert(MPI_Bcast(fv, fc, MPI_FLOAT, 0, MPI_COMM_WORLD));
#else
    return 1;
#endif
}

int mpi_share_integer(int ic, int *iv)
{
#ifdef MPI
    return mpi_assert(MPI_Bcast(iv, ic, MPI_INTEGER, 0, MPI_COMM_WORLD));
#else
    return 1;
#endif
}

/*---------------------------------------------------------------------------*/

GLuint shared_load_program(const char *filename, GLenum target)
{
    struct stat buf;

    char *ptr = NULL;
    int   len = 0;

    GLuint program = 0;

    /* If this host is root, determine the file size. */

    if (mpi_isroot() && (stat(filename, &buf) == 0))
        len = buf.st_size + 1;
        
    /* Broadcast the size and allocate that much memory. */

    mpi_share_integer(1, &len);

    if ((ptr = (char *) calloc(1, len)))
    {
        FILE *fp;

        /* If this host is root, read the contents of the file. */

        if (mpi_isroot() && (fp = fopen(filename, "r")))
        {
            fread(ptr, 1, len, fp);
            fclose(fp);
        }

        /* Broadcast the contents of the file. */

        mpi_share_char(len, ptr);

        /* Generate and initialize a program object. */

        glGenProgramsARB(1, &program);
        glBindProgramARB(target, program);
        glProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB, len - 1, ptr);

        /* If this host is root, report any error in the program text. */

        if (mpi_isroot() && (glGetError() != GL_NO_ERROR))
        {
            const GLubyte *msg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
            fprintf(stderr, "%s", (const char *) msg);
        }

        free(ptr);
    }
    return program;
}

GLuint shared_load_texture(const char *filename, int *width, int *height)
{
    GLubyte *p = NULL;
    int      w = 0;
    int      h = 0;
    int      b = 0;

    GLuint texture = 0;

    /* If this host is root, load the image file. */

    if (mpi_isroot()) p = (GLubyte *) image_load_png(filename, &w, &h, &b);

    /* Broadcast the image attributes. */

    mpi_share_integer(1, &w);
    mpi_share_integer(1, &h);
    mpi_share_integer(1, &b);

    /* If this host is not root, allocate a pixel buffer. */

    if (!mpi_isroot()) p = (GLubyte *) malloc(w * h * b);

    /* Broadcast the pixel data. */

    mpi_share_byte(w * h * b, p);

    /* Create a texture object. */

    texture = image_make_tex(p, w, h, b);
    *width  = w;
    *height = h;

    free(p);

    return texture;
}

/*---------------------------------------------------------------------------*/

const char *event_string(int type)
{
    switch (type)
    {
    case EVENT_DRAW:          return "draw";
    case EVENT_EXIT:          return "exit";

    case EVENT_ENTITY_CREATE: return "entity create";
    case EVENT_ENTITY_PARENT: return "entity parent";
    case EVENT_ENTITY_DELETE: return "entity delete";
    case EVENT_ENTITY_MOVE:   return "entity move";
    case EVENT_ENTITY_TURN:   return "entity turn";
    case EVENT_ENTITY_SIZE:   return "entity size";

    case EVENT_CAMERA_CREATE: return "camera create";
    case EVENT_SPRITE_CREATE: return "sprite create";
    case EVENT_OBJECT_CREATE: return "object create";
    case EVENT_LIGHT_CREATE:  return "light create";
    case EVENT_PIVOT_CREATE:  return "pivot create";

    case EVENT_CAMERA_DIST:   return "camera dist";
    case EVENT_CAMERA_ZOOM:   return "camera zoom";
    }

    return "UNKNOWN";
}

/*---------------------------------------------------------------------------*/

