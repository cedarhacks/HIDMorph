local STICK_MAX = 127;

local w_down = false;
local a_down = false;
local s_down = false;
local d_down = false;

local i_down = false;
local k_down = false;
local j_down = false;
local l_down = false;

local n_down = false;
local m_down = false;
local comma_down = false;
local space_down = false;

local enter_down = false;
local apastr_down = false;

local out_buttons = 0;
local out_x = 0;
local out_y = 0;
local out_rx = 0;
local out_ry = 0;
local out_hat = 0;

local function update_buttons()
    local b = 0;

    -- 0x0001 = B
    -- 0x0002 = A
    -- 0x0004 = none
    -- 0x0008 = Y

    -- 0x0010 = X
    -- 0x0020 = none
    -- 0x0040 = lbumper
    -- 0x0080 = rbumper

    -- 0x0100 = none
    -- 0x0200 = none
    -- 0x0400 = back
    -- 0x0800 = start

    -- 0x1000 = home
    -- 0x2000 = ljoystick
    -- 0x4000 = rjoystick
    -- 0x8000 = none

    if n_down then b = b | GAMEPAD_BTN.Y end
    if m_down then b = b | GAMEPAD_BTN.A end
    if comma_down then b = b | GAMEPAD_BTN.X end
    if space_down then b = b | GAMEPAD_BTN.B end

    if enter_down then b = b | GAMEPAD_BTN.START end
    if apastr_down then b = b | GAMEPAD_BTN.BACK end

    out_buttons = b;
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
    elseif keycode == KEY.N then
        n_down = pressed
    elseif keycode == KEY.M then
        m_down = pressed
    elseif keycode == KEY.COMMA then
        comma_down = pressed
    elseif keycode == KEY.SPACE then
        space_down = pressed
    elseif keycode == KEY.ENTER then
        enter_down = pressed
    elseif keycode == KEY.APOSTROPHE then
        apastr_down = pressed
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
