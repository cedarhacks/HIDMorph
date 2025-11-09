
print("-> [LUA]  initializing lua test")
local mouse_good = init_hid_mouse();
print("-> [LUA]  done initalize hid mouse ", mouse_good)

local x = 0;
local y = 0;
local i = 0.0;
local cr = 2;

if mouse_good then
    
    while 1 do
        
        i = i + 0.1;
        set_mouse_pos(x,y);
        x = cr*math.sin(i);
        y = cr*math.cos(i);
    end
end