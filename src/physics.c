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

#include <ode/ode.h>

/*---------------------------------------------------------------------------*/

static dWorldID      world;
static dSpaceID      space;
static dJointGroupID group;

#define MAX_CONTACTS 4

/*---------------------------------------------------------------------------*/

static void callback(void *data, dGeomID o1, dGeomID o2)
{
    dContact contact[MAX_CONTACTS];
    size_t sz = sizeof (dContact);
    int i, n;

    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    /* If the two bodies are connected, do nothing. */

    if (b1 && b2 && dAreConnectedExcluding(b1, b1, dJointTypeContact))
        return;
    
    /* Initialize the surface properties for each contact. */

    for (i=0; i<MAX_CONTACTS; ++i)
    {
        contact[i].surface.mode       = dContactBounce | dContactSoftCFM;
        contact[i].surface.mu         = dInfinity;
        contact[i].surface.mu2        = 0.00;
        contact[i].surface.bounce     = 0.10;
        contact[i].surface.bounce_vel = 0.10;
        contact[i].surface.soft_cfm   = 0.01;
    }

    /* Enumerate the collision points, creating a contact joint for each. */

    if ((n = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sz)))
        for (i = 0; i < n; ++i)
        {
            dJointID c = dJointCreateContact(world, group, contact + i);
            dJointAttach(c, b1, b2);
        }
}

void physics_step(float dt)
{
    dSpaceCollide(space, 0, callback);
    dWorldQuickStep(world, 0.005);
}

/*===========================================================================*/

static void set_rotation(dMatrix3 D, const float S[16])
{
    D[0] = S[0]; D[1] = S[4]; D[2]  = S[8];
    D[4] = S[1]; D[5] = S[5]; D[6]  = S[9];
    D[8] = S[2]; D[9] = S[6]; D[10] = S[10];
}

static void get_rotation(float D[16], const dMatrix3 S)
{
    D[0] = S[0]; D[4] = S[1]; D[8]  = S[2];  D[12] = 0;
    D[1] = S[4]; D[5] = S[5]; D[9]  = S[6];  D[13] = 0;
    D[2] = S[8]; D[6] = S[9]; D[10] = S[10]; D[14] = 0;
    D[3] =    0; D[7] =    0; D[11] =     0; D[16] = 1;   
}

/*---------------------------------------------------------------------------*/

dBodyID create_physics_body(void)
{
    return dBodyCreate(world);
}

dGeomID create_physics_box(dBodyID body, const float bound[6])
{
    float dx = bound[3] - bound[0];
    float dy = bound[4] - bound[1];
    float dz = bound[5] - bound[2];
    dMass m;

    dGeomID geom = dCreateBox(space, dx, dy, dz);

    dMassSetBox(&m, 5, dx, dy, dz);

    dBodySetMass(body, &m);
    dGeomSetBody(geom, body);

    return geom;
}

/*---------------------------------------------------------------------------*/

void set_physics_position(dGeomID geom, const float p[3])
{
    if (geom) dGeomSetPosition(geom, p[0], p[1], p[2]);
}

void get_physics_position(dGeomID geom, float p[3])
{
    if (geom)
    {
        p[0] = dGeomGetPosition(geom)[0];
        p[1] = dGeomGetPosition(geom)[1];
        p[2] = dGeomGetPosition(geom)[2];

        printf("%f %f %f\n", p[0], p[1], p[2]);
    }
}

/*---------------------------------------------------------------------------*/

void set_physics_rotation(dGeomID geom, const float r[16])
{
    dMatrix3 R;

    if (geom)
    {
        set_rotation(R, r);
        dGeomSetRotation(geom, R);
    }
}

void get_physics_rotation(dGeomID geom, float r[16])
{
    if (geom) get_rotation(r, dGeomGetRotation(geom));
}

/*===========================================================================*/

int startup_physics(void)
{
    world = dWorldCreate();
    space = dHashSpaceCreate(0);

    dWorldSetGravity(world, 0, -9.0, 0);
    dWorldSetAutoDisableFlag(world, 1);

    return 1;
}
