
-------------------------------------------------------------------------------

pnt_x  = 0.0
pnt_y  = 0.0

rot_x  = 0.0
rot_y  = 0.0

music  = nil
sound  = nil

persp  = nil
ortho  = nil
obj1   = nil
obj2   = nil
light  = nil
point  = nil
btn    = { }

-------------------------------------------------------------------------------

function init_2D()
    ortho = E.create_camera(E.camera_type_orthogonal)
    point = E.create_sprite("head.png")

    E.entity_alpha(point, 0.5)

    E.entity_parent(point, ortho)
    E.entity_scale(point, 0.5, 0.5, 0.5)
end

function init_3D()
    persp = E.create_camera(E.camera_type_perspective)
    obj1  = E.create_object("ball.obj")
    obj2  = E.create_object("ball.obj")
    light = E.create_light(E.light_type_positional)
    pivot = E.create_pivot()

    E.entity_parent(light, persp)
    E.entity_parent(pivot, light)
    E.entity_parent(obj1,  pivot)
    E.entity_parent(obj2,  pivot)

    E.entity_alpha(obj1, 0.5)
    E.entity_flag(obj2, E.entity_flag_billboard, true)

    E.camera_zoom(persp,  0.001)
    E.camera_dist(persp, 10.000)

    E.entity_position(light, 0, 2, 2)
    E.entity_position(obj1, -1, 0, 0)
    E.entity_position(obj2,  1, 0, 0)
end

function do_start()
    sound = E.sound_load("fizzle.ogg")
    music = E.sound_load("inter.ogg")

    init_2D()
    init_3D()

    E.sound_loop(music);

    E.enable_idle(true)

    return true
end

function do_click(b, s)
   btn[b] = s

   if b == 1 and s then
       E.sound_play(sound)
   end
   if b == 2 and s then
       E.sound_stop(sound)
   end

   return false
end

function do_timer(dt)
    rot_y = rot_y + dt * 90

    if (pivot) then
        E.entity_rotation(pivot, rot_x, rot_y, 0)
    end

    return true
end

function do_point(dx, dy)
    pnt_x = pnt_x + dx
    pnt_y = pnt_y - dy

    if (point) then
        E.entity_position(point, pnt_x, pnt_y, 0)
    end

    if btn[1] then
        rot_x = rot_x + dy
        rot_y = rot_y + dx

        if (pivot) then
            E.entity_rotation(pivot, rot_x, rot_y, 0)
        end

        return true
    end

    return true
end

-------------------------------------------------------------------------------
