
-------------------------------------------------------------------------------

w  = 1280
h  = 1024
dx = w + 384  -- Tile width including mullion
dy = h + 384  -- Tile height including mullion

global_w  =  dx * 5
global_y  =  dy * 3
global_x  = -global_w / 2
global_y  = -global_y / 2

add_tile("yorda1-10",   0, 0, global_x,          global_y,          w, h)
add_tile("yorda2-10",   0, 0, global_x,          global_y + dy,     w, h)
add_tile("yorda3-10",   0, 0, global_x,          global_y + dy * 2, w, h)
add_tile("yorda4-10",   0, 0, global_x + dx,     global_y,          w, h)
add_tile("yorda5-10",   0, 0, global_x + dx,     global_y + dy,     w, h)
add_tile("yorda6-10",   0, 0, global_x + dx,     global_y + dy * 2, w, h)
add_tile("yorda7-10",   0, 0, global_x + dx * 2, global_y,          w, h)
add_tile("yorda8-10",   0, 0, global_x + dx * 2, global_y + dy,     w, h)
add_tile("yorda9-10",   0, 0, global_x + dx * 2, global_y + dy * 2, w, h)
add_tile("yorda10-10",  0, 0, global_x + dx * 4, global_y,          w, h)
add_tile("yorda11-10",  0, 0, global_x + dx * 4, global_y + dy,     w, h)
add_tile("yorda12-10",  0, 0, global_x + dx * 4, global_y + dy * 2, w, h)
add_tile("yorda13-10",  0, 0, global_x + dx * 5, global_y,          w, h)
add_tile("yorda14-10",  0, 0, global_x + dx * 5, global_y + dy,     w, h)
add_tile("yorda15-10",  0, 0, global_x + dx * 5, global_y + dy * 2, w, h)

-------------------------------------------------------------------------------

