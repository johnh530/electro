
-------------------------------------------------------------------------------

camera = nil
galaxy = nil

zoom =   1.0
magn = 100.0
dist =   2.0
spin =   0.0

setzoom = false
setmagn = false
setdist = false

-------------------------------------------------------------------------------

function do_start()
    camera = E.create_camera(E.camera_type_perspective)
    galaxy = E.create_galaxy("../hip_main.dat", "../tyc2.dat")

    E.entity_parent(galaxy, camera)

    E.entity_position(camera, 0, 15.5, 9200)

    E.camera_zoom(camera, zoom)
    E.camera_dist(camera, dist)
    E.galaxy_magn(galaxy, magn)
end

function do_timer()
    local x, y, z = E.entity_get_rotation(camera)

    E.entity_rotation(camera, x, y + spin, z)
    return true
end

function do_point(dx, dy)
    if setzoom then      -- Set the camera zoom.

        zoom = zoom + dy * 0.01
        if zoom < 0.001 then
            zoom = 0.001
        end

        E.camera_zoom(camera, zoom * zoom)

    elseif setmagn then  -- Set the stellar magnitude multiplier.

        magn = magn - dy * 1.0
        if magn < 0 then
            magn = 0
        end

        E.galaxy_magn(galaxy, magn)

    elseif setdist then  -- Set the camera distance from the center.

        dist = dist + dy * 0.1
        if dist < 0 then
            dist = 0
        end

        E.camera_dist(camera, dist)

    else                 -- None of the above.  Just pan the camera

        local x, y, z = E.entity_get_rotation(camera)

        x = x - dy * 0.1 * zoom * zoom
        y = y - dx * 0.1 * zoom * zoom

        if x < -90 then x = -90 end
        if x >  90 then x =  90 end

        if y < -180 then y = y + 360 end
        if y >  180 then y = y - 360 end

        E.entity_rotation(camera, x, y, z)
    end

    return true
end

function do_click(b, s)
    if b == 1 then setzoom = s end
    if b == 2 then setmagn = s end
    if b == 3 then setdist = s end
end

-------------------------------------------------------------------------------

