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
#include <math.h>

#include "opengl.h"
#include "vector.h"
#include "frustum.h"
#include "display.h"
#include "physics.h"
#include "matrix.h"
#include "buffer.h"
#include "entity.h"
#include "camera.h"
#include "sprite.h"
#include "object.h"
#include "string.h"
#include "galaxy.h"
#include "light.h"
#include "pivot.h"
#include "event.h"
#include "utility.h"
#include "tracker.h"

/*---------------------------------------------------------------------------*/

struct entity
{
    int type;
    int data;
    int flags;
    int state;

    /* Entity transformation. */

    float rotation[16];
    float position[3];
    float scale[3];
    float bound[6];
    float alpha;

    /* Entity hierarchy. */

    int car;
    int cdr;
    int par;

    /* Physical representations. */

    dBodyID body;
    dGeomID geom;
};

static vector_t            entity;
static struct entity_func *entity_func[TYPE_COUNT];

/*---------------------------------------------------------------------------*/

#define CAR(i) ((struct entity *) vecget(entity, (i)))->car
#define CDR(i) ((struct entity *) vecget(entity, (i)))->cdr
#define PAR(i) ((struct entity *) vecget(entity, (i)))->par

static struct entity *get_entity(int i)
{
    return (struct entity *) vecget(entity, i);
}

static int new_entity(void)
{
    int i, n = vecnum(entity);

    for (i = 0; i < n; ++i)
        if (get_entity(i)->type == 0)
            return i;

    return vecadd(entity);
}

/*===========================================================================*/

int entity_data(int i)
{
    return get_entity(i)->data;
}

int entity_type(int i)
{
    return get_entity(i)->type;
}


const char *entity_name(int i)
{
    struct entity *e = get_entity(i);

    if (entity_func[e->type])
        return entity_func[e->type]->name;
    else
        return "unknown";
}

/*---------------------------------------------------------------------------*/

void transform_camera(int i)
{
    struct entity *e = get_entity(i);

    float M[16];

    /* Inverse scale. */

    glScalef(1 / e->scale[0],
             1 / e->scale[1],
             1 / e->scale[2]);

    /* Inverse (transposed) rotation. */

    load_xps(M, e->rotation);
    glMultMatrixf(M);

    /* Inverse translation. */

    glTranslatef(-e->position[0],
                 -e->position[1],
                 -e->position[2]);
}

void transform_entity(int i)
{
    struct entity *e = get_entity(i);

    /* Translation. */

    glTranslatef(e->position[0],
                 e->position[1],
                 e->position[2]);

    /* Rotation. */

    glMultMatrixf(e->rotation);

    /* Billboard. */

    if (e->flags & FLAG_BILLBOARD)
    {
        float M[16];

        glGetFloatv(GL_MODELVIEW_MATRIX, M);

        M[0] = 1; M[4] = 0; M[ 8] = 0;
        M[1] = 0; M[5] = 1; M[ 9] = 0;
        M[2] = 0; M[6] = 0; M[10] = 1;

        glLoadMatrixf(M);
    }

    /* Scale. */

    glScalef(e->scale[0],
             e->scale[1],
             e->scale[2]);
}

/*---------------------------------------------------------------------------*/

static void init_entity(int i)
{
    struct entity *e = get_entity(i);

    if (e->state == 0)
    {
        /* Find the bounding box of this entity, if any. */

        if (entity_func[e->type]       &&
            entity_func[e->type]->bbox &&
            entity_func[e->type]->bbox(e->data, e->bound))
            e->flags |= FLAG_BOUNDED;

        e->state = 1;
    }
}

static void fini_entity(int i)
{
    struct entity *e = get_entity(i);

    if (e->state == 1)
    {
        e->state = 0;
    }
}

/*---------------------------------------------------------------------------*/

int test_entity_bbox(int i)
{
    struct entity *e = get_entity(i);
    struct frustum F;

    if (e->flags & FLAG_BOUNDED)
    {
        get_frustum(&F);
        return tst_frustum(&F, e->bound);
    }

    return 1;
}

void draw_entity_tree(int i, int f, float a)
{
    int j;

    /* Traverse the hierarchy.  Iterate the child list of this entity. */

    for (j = CAR(i); j; j = CDR(j))
    {
        struct entity *e = get_entity(j);

        if ((e->flags & FLAG_HIDDEN) == 0)
        {
            init_entity(j);

            glPushAttrib(GL_ENABLE_BIT  |
                         GL_POLYGON_BIT |
                         GL_DEPTH_BUFFER_BIT);
            {
                /* Enable wireframe if specified. */

                if (e->flags & FLAG_WIREFRAME)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                /* Enable line smoothing if requested. */

                if (e->flags & FLAG_LINE_SMOOTH)
                    glEnable(GL_LINE_SMOOTH);

                /* Draw this entity. */

                if (f & DRAW_VARRIER_TEXGEN)
                    set_texture_coordinates();

                if (entity_func[e->type] &&
                    entity_func[e->type]->draw)
                    entity_func[e->type]->draw(j, e->data, f, a);
            }
            glPopAttrib();
        }
    }
}

/*===========================================================================*/

static void detach_entity(int i)
{
    /* Never allow the root entity to be used as a child. */

    if (i)
    {
        int j, k, p = PAR(i);

        /* Remove the child from its parent's child list. */

        for (j = 0, k = CAR(p); k; j = k, k = CDR(k))
            if (k == i)
            {
                if (j)
                    CDR(j) = CDR(k);
                else
                    CAR(p) = CDR(k);
            }
    }
}

static void attach_entity(int i, int j)
{
    /* Never allow the root entity to be used as a child. */

    if (i && i != j)
    {
        /* Insert the child into the new parent's child list. */

        PAR(i) = j;
        CDR(i) = CAR(j);
        CAR(j) = i;
    }
}

/*===========================================================================*/

static void create_entity(int i, int type, int data)
{
    struct entity *e = get_entity(i);

    load_idt(e->rotation);

    e->type     = type;
    e->data     = data;
    e->flags    = 0;
    e->scale[0] = 1;
    e->scale[1] = 1;
    e->scale[2] = 1;
    e->alpha    = 1;

    attach_entity(i, 0);

    if (entity_func[type]       &&
        entity_func[type]->bbox &&
        entity_func[type]->bbox(data, e->bound))
        e->flags |= FLAG_BOUNDED;
}

int send_create_entity(int type, int data)
{
    int i;

    if ((i = new_entity()) >= 0)
    {
        struct entity *e = get_entity(i);

        send_index(type);
        send_index(data);
    
        create_entity(i, type, data);

        if (type == TYPE_OBJECT)
        {
            e->body = create_physics_body();
            e->geom = create_physics_box(e->body, e->bound);
        }

        return i;
    }
    return -1;
}

void recv_create_entity(void)
{
    int i    = new_entity();
    int type = recv_index();
    int data = recv_index();

    create_entity(i, type, data);
}

/*---------------------------------------------------------------------------*/

void send_parent_entity(int i, int j)
{
    send_event(EVENT_PARENT_ENTITY);
    send_index(i);
    send_index(j);

    detach_entity(i);
    attach_entity(i, j);
}

void recv_parent_entity(void)
{
    int i = recv_index();
    int j = recv_index();

    detach_entity(i);
    attach_entity(i, j);
}

/*---------------------------------------------------------------------------*/

void send_set_entity_rotation(int i, const float r[3])
{
    float M[16];

    if (get_entity(i)->type == TYPE_CAMERA)
    {
        load_rot_mat(M, 0, 0, 1, r[2]);
        mult_rot_mat(M, 0, 1, 0, r[1]);
        mult_rot_mat(M, 1, 0, 0, r[0]);
    }
    else
    {
        load_rot_mat(M, 1, 0, 0, r[0]);
        mult_rot_mat(M, 0, 1, 0, r[1]);
        mult_rot_mat(M, 0, 0, 1, r[2]);
    }

    send_set_entity_basis(i, M);
}

void send_set_entity_position(int i, const float p[3])
{
    struct entity *e = get_entity(i);

    send_event(EVENT_SET_ENTITY_POSITION);
    send_index(i);

    send_float((e->position[0] = p[0]));
    send_float((e->position[1] = p[1]));
    send_float((e->position[2] = p[2]));

    set_physics_position(e->geom, e->position);
}

void send_set_entity_basis(int i, const float M[16])
{
    struct entity *e = get_entity(i);

    send_event(EVENT_SET_ENTITY_BASIS);
    send_index(i);

    send_float(M[0]);
    send_float(M[1]);
    send_float(M[2]);

    send_float(M[4]);
    send_float(M[5]);
    send_float(M[6]);

    send_float(M[8]);
    send_float(M[9]);
    send_float(M[10]);

    load_mat(e->rotation, M);
    set_physics_rotation(e->geom, e->rotation);
}

void send_set_entity_scale(int i, const float v[3])
{
    struct entity *e = get_entity(i);

    send_event(EVENT_SET_ENTITY_SCALE);
    send_index(i);

    send_float((e->scale[0] = v[0]));
    send_float((e->scale[1] = v[1]));
    send_float((e->scale[2] = v[2]));
}

void send_set_entity_bound(int i, const float v[3])
{
    struct entity *e = get_entity(i);

    send_event(EVENT_SET_ENTITY_BOUND);
    send_index(i);

    send_float((e->bound[0] = v[0]));
    send_float((e->bound[1] = v[1]));
    send_float((e->bound[2] = v[2]));
    send_float((e->bound[3] = v[3]));
    send_float((e->bound[4] = v[4]));
    send_float((e->bound[5] = v[5]));
}

void send_set_entity_alpha(int i, float a)
{
    struct entity *e = get_entity(i);

    send_event(EVENT_SET_ENTITY_ALPHA);
    send_index(i);

    send_float((e->alpha = a));
}

void send_set_entity_flags(int i, int flags, int state)
{
    struct entity *e = get_entity(i);

    send_event(EVENT_SET_ENTITY_FLAGS);
    send_index(i);
    send_index(flags);
    send_index(state);

    if (state)
        e->flags = e->flags | ( flags);
    else
        e->flags = e->flags & (~flags);
}

/*---------------------------------------------------------------------------*/

void send_move_entity(int i, const float v[3])
{
    struct entity *e = get_entity(i);

    float p[3];

    p[0] = e->position[0] + (e->rotation[0]  * v[0] +
                             e->rotation[4]  * v[1] +
                             e->rotation[8]  * v[2]);
    p[1] = e->position[1] + (e->rotation[1]  * v[0] +
                             e->rotation[5]  * v[1] +
                             e->rotation[9]  * v[2]);
    p[2] = e->position[2] + (e->rotation[2]  * v[0] +
                             e->rotation[6]  * v[1] +
                             e->rotation[10] * v[2]);

    send_set_entity_position(i, p);
}

void send_turn_entity(int i, const float r[3])
{
    struct entity *e = get_entity(i);

    float M[16], R[16];

    /* Compose a transformation matrix. */

    load_rot_mat(M, e->rotation[0],
                    e->rotation[1],
                    e->rotation[2], r[0]);
    mult_rot_mat(M, e->rotation[4],
                    e->rotation[5],
                    e->rotation[6], r[1]);
    mult_rot_mat(M, e->rotation[8],
                    e->rotation[9],
                    e->rotation[10], r[2]);

    /* Transform the entity's basis. */

    load_idt(R);

    mult_mat_vec(R + 0, M, e->rotation + 0);
    mult_mat_vec(R + 4, M, e->rotation + 4);
    mult_mat_vec(R + 8, M, e->rotation + 8);

    /* Re-orthogonalize the basis. */

    cross(R + 8, R + 0, R + 4);
    cross(R + 4, R + 8, R + 0);
    cross(R + 0, R + 4, R + 8);

    /* Re-normalize the basis. */

    normalize(R + 0);
    normalize(R + 4);
    normalize(R + 8);

    send_set_entity_basis(i, R);
}

/*---------------------------------------------------------------------------*/

void recv_set_entity_position(void)
{
    struct entity *e = (struct entity *) vecget(entity, recv_index());

    e->position[0] = recv_float();
    e->position[1] = recv_float();
    e->position[2] = recv_float();
}

void recv_set_entity_basis(void)
{
    struct entity *e = (struct entity *) vecget(entity, recv_index());

    load_idt(e->rotation);

    e->rotation[0]  = recv_float();
    e->rotation[1]  = recv_float();
    e->rotation[2]  = recv_float();

    e->rotation[4]  = recv_float();
    e->rotation[5]  = recv_float();
    e->rotation[6]  = recv_float();

    e->rotation[8]  = recv_float();
    e->rotation[9]  = recv_float();
    e->rotation[10] = recv_float();
}

void recv_set_entity_scale(void)
{
    struct entity *e = (struct entity *) vecget(entity, recv_index());

    e->scale[0] = recv_float();
    e->scale[1] = recv_float();
    e->scale[2] = recv_float();
}

void recv_set_entity_bound(void)
{
    struct entity *e = (struct entity *) vecget(entity, recv_index());

    e->bound[0] = recv_float();
    e->bound[1] = recv_float();
    e->bound[2] = recv_float();
    e->bound[3] = recv_float();
    e->bound[4] = recv_float();
    e->bound[5] = recv_float();
}

void recv_set_entity_alpha(void)
{
    struct entity *e = (struct entity *) vecget(entity, recv_index());

    e->alpha = recv_float();
}

void recv_set_entity_flags(void)
{
    struct entity *e = (struct entity *) vecget(entity, recv_index());

    int flags = recv_index();
    int state = recv_index();

    if (state)
        e->flags = e->flags | ( flags);
    else
        e->flags = e->flags & (~flags);
}

/*---------------------------------------------------------------------------*/

void get_entity_position(int i, float p[3])
{
    struct entity *e = get_entity(i);

    p[0] = e->position[0];
    p[1] = e->position[1];
    p[2] = e->position[2];
}

void get_entity_x_vector(int i, float v[3])
{
    struct entity *e = get_entity(i);

    v[0] = e->rotation[0];
    v[1] = e->rotation[1];
    v[2] = e->rotation[2];
}

void get_entity_y_vector(int i, float v[3])
{
    struct entity *e = get_entity(i);

    v[0] = e->rotation[4];
    v[1] = e->rotation[5];
    v[2] = e->rotation[6];
}

void get_entity_z_vector(int i, float v[3])
{
    struct entity *e = get_entity(i);

    v[0] = e->rotation[8];
    v[1] = e->rotation[9];
    v[2] = e->rotation[10];
}

void get_entity_scale(int i, float v[3])
{
    struct entity *e = get_entity(i);

    v[0] = e->scale[0];
    v[1] = e->scale[1];
    v[2] = e->scale[2];
}

void get_entity_bound(int i, float v[6])
{
    struct entity *e = get_entity(i);

    v[0] = e->bound[0];
    v[1] = e->bound[1];
    v[2] = e->bound[2];
    v[3] = e->bound[3];
    v[4] = e->bound[4];
    v[5] = e->bound[5];
}

float get_entity_alpha(int i)
{
    return get_entity(i)->alpha;
}

int get_entity_flags(int i)
{
    return get_entity(i)->flags;
}

/*---------------------------------------------------------------------------*/

static void create_clone(int i, int j)
{
    struct entity *e = get_entity(i);

    if (entity_func[e->type] &&
        entity_func[e->type]->dupe)
        entity_func[e->type]->dupe(e->data);

    create_entity(j, e->type, e->data);
}

int send_create_clone(int i)
{
    int j;

    if ((j = new_entity()) >= 0)
    {
        send_event(EVENT_CREATE_CLONE);
        send_index(i);

        create_clone(i, j);
    }
    return j;
}

void recv_create_clone(void)
{
    create_clone(recv_index(), new_entity());
}

/*---------------------------------------------------------------------------*/

static void free_entity(int i)
{
    struct entity *e = get_entity(i);

    fini_entity(i);

    /* Delete all child entities. */

    while (e->car)
        free_entity(e->car);

    /* Remove this entity from the parent's child list. */

    detach_entity(i);

    /* Delete the type-specific data. */

    if (entity_func[e->type] &&
        entity_func[e->type]->free)
        entity_func[e->type]->free(e->data);

    /* Pave it. */

    memset(e, 0, sizeof (struct entity));
}

void send_delete_entity(int i)
{
    send_event(EVENT_DELETE_ENTITY);
    send_index(i);

    free_entity(i);
}

void recv_delete_entity(void)
{
    free_entity(recv_index());
}

/*---------------------------------------------------------------------------*/

int get_entity_parent(int i)
{
    return PAR(i);
}

int get_entity_child(int i, int n)
{
    int j, m;

    for (m = 0, j = CAR(i); j; m++, j = CDR(j))
        if (n == m)
            return j;

    return -1;
}

/*===========================================================================*/

void draw_entities(void)
{
    glLoadIdentity();

    /* Begin traversing the scene graph at the root. */

    draw_entity_tree(0, 0, 1);
    opengl_check("draw_entities");
}

void step_entities(float dt)
{
    float r[2][3];
    float p[2][3];
    float M[16];

    int i, n = vecnum(entity);

    /* Acquire tracking info. */

    get_tracker_position(0, p[0]);
    get_tracker_position(1, p[1]);
    get_tracker_rotation(0, r[0]);
    get_tracker_rotation(1, r[1]);

    load_rot_mat(M, 1, 0, 0, r[0][0]);
    mult_rot_mat(M, 0, 1, 0, r[0][1]);
    mult_rot_mat(M, 0, 0, 1, r[0][2]);

    /* Distribute it to all cameras and tracked entities. */

    for (i = 0; i < n; ++i)
    {
        struct entity *e = get_entity(i);

        if (e->type == TYPE_CAMERA)
            send_set_camera_offset(e->data, p[0], M);

        else if (e->type)
        {
            if (e->flags & FLAG_POS_TRACKED_0)
                send_set_entity_position(i, p[0]);
            if (e->flags & FLAG_POS_TRACKED_1)
                send_set_entity_position(i, p[1]);

            if (e->flags & FLAG_ROT_TRACKED_0)
                send_set_entity_rotation(i, r[0]);
            if (e->flags & FLAG_ROT_TRACKED_1)
                send_set_entity_rotation(i, r[1]);
        }
    }

    /* Run the physical simulation and update all entity states. */

    physics_step(dt);

    for (i = 0; i < n; ++i)
    {
        struct entity *e = get_entity(i);

        if (e->type)
        {
            get_physics_position(e->geom, e->position);
            get_physics_rotation(e->geom, e->rotation);
/*
            float p[3], R[16];

            get_physics_position(e->geom, p);
            get_physics_rotation(e->geom, R);

            send_set_entity_position(i, p);
            send_set_entity_basis   (i, R);
*/
        }
    }
}

/*---------------------------------------------------------------------------*/

void init_entities(void)
{
    int i, n = vecnum(entity);

    /* Ask all entities with GL state to initialize themselves. */

    for (i = 0; i < n; ++i)
    {
        struct entity *e = get_entity(i);

        if (e->type)
        {
            init_entity(i);

            if (entity_func[e->type] &&
                entity_func[e->type]->init)
                entity_func[e->type]->init(e->data);
        }
    }
}

void fini_entities(void)
{
    int i, n = vecnum(entity);

    /* Ask all entities with GL state to finalize themselves. */

    for (i = 0; i < n; ++i)
    {
        struct entity *e = get_entity(i);

        if (e->type)
        {
            fini_entity(i);

            if (entity_func[e->type] &&
                entity_func[e->type]->fini)
                entity_func[e->type]->fini(e->data);
        }
    }
}

/*===========================================================================*/

int startup_entity(void)
{
    if ((entity = vecnew(256, sizeof (struct entity))))
    {
        struct entity *e = (struct entity *) vecget(entity, vecadd(entity));

        e->type = TYPE_ROOT;

        entity_func[TYPE_NULL]   = NULL;
        entity_func[TYPE_ROOT]   = NULL;
        entity_func[TYPE_CAMERA] = startup_camera();
        entity_func[TYPE_SPRITE] = startup_sprite();
        entity_func[TYPE_OBJECT] = startup_object();
        entity_func[TYPE_STRING] = startup_string();
        entity_func[TYPE_GALAXY] = startup_galaxy();
        entity_func[TYPE_LIGHT]  = startup_light();
        entity_func[TYPE_PIVOT]  = startup_pivot();

        return 1;
    }
    return 0;
}

