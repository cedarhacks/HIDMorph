local STICK_MAX = 127;

local w_down = false;
local a_down = false;
local s_down = false;
local d_down = false;

local i_down = false;
local k_down = false;
local j_down = false;
local l_down = false;

local out_buttons = 0;
local out_x = 0;
local out_y = 0;
local out_rx = 0;
local out_ry = 0;
local out_hat = 0;

local function update_buttons()
end

local function update_sticks()
    out_x = 0;
    out_y = 0;
    out_rx = 0;
    out_ry = 0;

    if a_down and not d_down then
        out_x = -STICK_MAX;
    elseif d_down and not a_down then
        out_x = STICK_MAX;    
    end

    if w_down and not s_down then
        out_y = -STICK_MAX;
    elseif s_down and not w_down then
        out_y = STICK_MAX;    
    end

    if i_down and not k_down then
        out_ry = -STICK_MAX;
    elseif k_down and not i_down then
        out_ry = STICK_MAX;    
    end

    if j_down and not l_down then
        out_rx = -STICK_MAX;
    elseif l_down and not j_down then
        out_rx = STICK_MAX;    
    end

end

register_mouse_callback(function(buttons, dx, dy, wheel)
    -- print("mouse:", buttons, dx, dy, wheel)
end)

register_keyboard_callback(function(keycode, pressed)

    if keycode == KEY.W then
        w_down = pressed;
    elseif keycode == KEY.A then
        a_down = pressed;
    elseif keycode == KEY.S then
        s_down = pressed;
    elseif keycode == KEY.D then
        d_down = pressed;
    elseif keycode == KEY.I then
        i_down = pressed;
    elseif keycode == KEY.J then
        j_down = pressed;
    elseif keycode == KEY.K then
        k_down = pressed;
    elseif keycode == KEY.L then
        l_down = pressed;
    end

    update_buttons();
    update_sticks();

end)

----------------------------------------------------------
-- MAIN LOOP
----------------------------------------------------------
while true do
    set_gamepad_raw(out_buttons, out_x, out_y, out_rx, out_ry, out_hat)
    wait_ms(1)
end
