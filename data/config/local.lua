
-------------------------------------------------------------------------------

--fp   = io.popen("hostname")
--name = fp:read()
--       fp:close()

name = "nico"

-------------------------------------------------------------------------------

w = 640
h = 480

global_x = -w
global_y = -h / 2
global_w =  w * 2
global_h =      2

E.add_tile(name, 0, 0, -w, -h / 2, w, h)
E.add_tile(name, w, 0,  0, -h / 2, w, h)

-------------------------------------------------------------------------------
