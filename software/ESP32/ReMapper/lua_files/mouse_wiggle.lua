print("-> [LUA]  initializing lua test")
local mouse_good = init_hid_mouse();
print("-> [LUA]  done initalize hid mouse ", mouse_good)

local x = 0;
local y = 0;

if mouse_good then
    while 1 do
        set_mouse_raw(0, x, y, 0);
        x = (math.random() * 20) - 10;
        y = (math.random() * 20) - 10;
    end
end
