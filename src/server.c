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

#include "opengl.h"
#include "joystick.h"
#include "viewport.h"
#include "console.h"
#include "buffer.h"
#include "script.h"
#include "entity.h"
#include "utility.h"
#include "sound.h"
#include "image.h"
#include "event.h"
#include "server.h"

/*---------------------------------------------------------------------------*/

static void server_draw(void);

static int server_grab = 0;
static int server_time = 0;

static float average_fps = 0.0f;

/*---------------------------------------------------------------------------*/

void grab(int b)
{
    if (b && !server_grab)
    {
        SDL_WM_GrabInput(SDL_GRAB_ON);
        SDL_ShowCursor(0);
    }
    if (!b && server_grab)
    {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        SDL_ShowCursor(1);
    }
    server_grab = b;
}

/*---------------------------------------------------------------------------*/

static Uint32 timer_callback(Uint32 interval, void *parameter)
{
    Uint32 t = SDL_GetTicks();
    SDL_Event e;

    /* On callback, push a user event giving time passed since last timer. */

    e.type      = SDL_USEREVENT;
    e.user.code = t - server_time;
    server_time = t;

    SDL_PushEvent(&e);

    /* Return the given interval to schedule a repeat timer event. */

    return interval;
}

static int         timer_on = 0;

void enable_timer(int b)
{
/*  static SDL_TimerID timer_id; */

    /* Enable or disable an SDL timer callback. */

    if (b && !timer_on)
    {
/*      timer_id = SDL_AddTimer(1000 / 40, timer_callback, NULL); */
        timer_on = 1;
        server_time = SDL_GetTicks();
    }

    if (!b && timer_on)
    {
        timer_on = 0;
/*      SDL_RemoveTimer(timer_id); */
    }
}

/*---------------------------------------------------------------------------*/

static void init_server(void)
{
    glViewport(0, 0, get_window_w(), get_window_h());

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

static void server_draw(void)
{
    const float gray[3] = { 0.2f, 0.2f, 0.2f };

    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    /* Draw the defined viewports to the stencil buffer. */

    glStencilFunc(GL_ALWAYS,   1, 0xFFFFFFFF);
    draw_viewport();

    /* Draw the mullions into the non-viewport parts of the frame buffer. */

    glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
    fill_viewport(gray, gray);

    /* Draw the scene into the viewport parts of the frame buffer. */

    glStencilFunc(GL_EQUAL,    1, 0xFFFFFFFF);

    draw_background();
    draw_entity();
    draw_console();

    /* Sync and swap. */

#ifdef MPI
    assert_mpi(MPI_Barrier(MPI_COMM_WORLD));
#endif
    SDL_GL_SwapBuffers();
}

static void server_perf(void)
{
    static int fps_old = 0;
    int        fps_new = (int) opengl_perf(&average_fps);

    if (fps_new != fps_old)
    {
        char buf[32];

        sprintf(buf, "%s - %d FPS", TITLE, fps_new);
        SDL_WM_SetCaption(buf, buf);

        fps_old = fps_new;
    }
}

/*---------------------------------------------------------------------------*/

static int server_keydn(SDL_KeyboardEvent *k)
{
    if (console_is_enabled())
        return input_console(k->keysym.sym, k->keysym.unicode);
    else
        return do_keyboard_script(k->keysym.sym, 1);
}

static int server_keyup(SDL_KeyboardEvent *k)
{
    if (console_is_enabled())
        return 0;
    else
        return do_keyboard_script(k->keysym.sym, 0);
}

/*---------------------------------------------------------------------------*/

static int server_loop(void)
{
    static int dirty = 1;
    static int count = 0;

    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        /* Handle point grab toggle. */

        if (e.type == SDL_KEYUP && e.key.keysym.sym == 27) grab(0);
        if (e.type == SDL_MOUSEBUTTONDOWN)                 grab(1);

        /* Handle console toggle. */

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F1)
            dirty = set_console_enable(!console_is_enabled());

        /* Dispatch the event to the scripting system. */

        if (server_grab)
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                dirty |= do_point_script(e.motion.xrel, e.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
                dirty |= do_click_script(e.button.button, 1);
                break;
            case SDL_MOUSEBUTTONUP:
                dirty |= do_click_script(e.button.button, 0);
                break;
            case SDL_JOYBUTTONDOWN:
                dirty |= do_joystick_script(e.jbutton.which,
                                            e.jbutton.button, 1);
                break;
            case SDL_JOYBUTTONUP:
                dirty |= do_joystick_script(e.jbutton.which,
                                            e.jbutton.button, 0);
                break;
            case SDL_KEYDOWN:
                dirty |= server_keydn(&e.key);
                break;
            case SDL_KEYUP:
                dirty |= server_keyup(&e.key);
                break;
            case SDL_USEREVENT:
                dirty |= do_timer_script(e.user.code);
                break;
            }

        /* Handle a clean exit.  TODO: remove redundancy. */

        if (e.type == SDL_QUIT)
        {
            pack_event(EVENT_EXIT);
            pack_event(EVENT_NULL);
            sync_buffer();

            return 0;
        }
    }

    /* Redraw a dirty buffer. */

    if (server_grab || dirty)
    {
        do_frame_script();

        if (dirty)
            pack_event(EVENT_DRAW);

        pack_event(EVENT_NULL);
        sync_buffer();

        if (dirty)
        {
            server_draw();
            server_perf();

            dirty = 0;
            count = count + 1;
        }
    }

    if (timer_on && server_grab) timer_callback(0, NULL);

    return 1;
}

/*---------------------------------------------------------------------------*/

void server(int argc, char *argv[])
{
    int argi;

    /*
    prep_tyc_galaxy();
    prep_hip_galaxy();
    */

    if (init_script())
    {
        init_console(CONSOLE_COLS, CONSOLE_ROWS);
        init_viewport();

        /* Read and execute all scripts given on the command line. */

        for (argi = 1; argi < argc; argi++)
            load_script(argv[argi], argi < argc - 1);

        sync_viewport();

        /* Initialize the main server window. */

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                     SDL_INIT_AUDIO | SDL_INIT_TIMER) == 0)
        {
            int w = get_window_w();
            int h = get_window_h();
            int m = SDL_OPENGL;

            /* Set some OpenGL framebuffer attributes. */

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            if (SDL_SetVideoMode(w, h, 0, m))
            {
                SDL_EnableUNICODE(1);

                /* Initialize all subsystems. */
	
                init_opengl();
                init_joystick();
                init_buffer();
                init_sound();
                init_image();
                init_server();
                init_entity();

                do_start_script();

                /* Block on SDL events.  Service them as they arrive. */

                SDL_PauseAudio(0);

                grab(1);

                while (SDL_WaitEvent(NULL))
                    if (server_loop() == 0)
                        break;

                /* Ensure everyone finishes all events before exiting. */

#ifdef MPI
                assert_mpi(MPI_Barrier(MPI_COMM_WORLD));
#endif
            }
            else fprintf(stderr, "%s\n", SDL_GetError());

            SDL_Quit();
        }
        else fprintf(stderr, "%s\n", SDL_GetError());
    }

    printf("Average FPS: %f\n", average_fps);
}

/*---------------------------------------------------------------------------*/
