eye_x =  2.50 / 12.0 * .5
eye_y = -1.0 / 12.0
eye_z =  -8.5 / 12.0

host = { }
tile = { }

-- 2 hosts: 1 server, 1 client

host[1]  = E.add_host("default",    0, 0, 2560, 1600)
host[2]  = E.add_host("default", 2560, 0, 2560, 1600)

-- Tile host numbers.

num = {
   host[1], host[2],
}

-- Tile sub-windows

w = {
   {      0,    0, 2560, 1600 },
   {      0,    0, 2560, 1600 },
}

-- Tile viewports.

v = {
   {      0,    0,    2560, 1600 },
   {   2735,    0,    2560, 1600 },
}

-- Tile screen locations.

p = {
    { -2.6755, 2.7787, -0.3022,  1.4879, 0.0, -1.4879, 0.0, 1.3177, 0.0 },
    { -1.0521, 2.7787, -1.8462,  2.1042, 0.0,  0.0,    0.0, 1.3177, 0.0 },
}

-- Varrier line screen definitions.
line_screen = {
    { 533.407, -8.479, 0.008, -.0018, 0.75 },
    { 533.407, -8.449, 0.008, -.00085, 0.75 },
}

-------------------------------------------------------------------------------

-- Configure all hosts.

for i = 1, 2 do

    tile[i] = E.add_tile(num[i], w[i][1], w[i][2], w[i][3], w[i][4])

    E.set_tile_viewport(tile[i], v[i][1], v[i][2], v[i][3], v[i][4])
    E.set_tile_position(tile[i], p[i][1], p[i][2], p[i][3], p[i][4],
                        p[i][5], p[i][6], p[i][7], p[i][8], p[i][9])
end

varrier_init()

-------------------------------------------------------------------------------
-- -1  0  0  0
--  0  1  0  0
--  0  0 -1  1.55
--  0  0  0  1
--
--D = 1.55
--E.set_tracker_transform(0, -1,  0,  0,  0,
--                            0,  1,  0,  0,
--                            0,  0, -1,  0,
--                            0,  0,  D,  1, 1, 0, 2)
--E.set_tracker_transform(1, -1,  0,  0,  0,
--                            0,  1,  0,  0,
--                            0,  0, -1,  0,
--                            0,  0,  D,  1, 1, 0, 2)
--
--
--E.set_tracker_transform(0, -1,  0,  0,  0,
--                            0,  1,  0,  0,
--                            0,  0, -1,  0,
--                            0,  0,  D,  1, 0, 1, 2)
--E.set_tracker_transform(1, -1,  0,  0,  0,
--                            0,  1,  0,  0,
--                            0,  0, -1,  0,
--                            0,  0,  D,  1, 0, 1, 2)
--
-------------------------------------------------------------------------------
