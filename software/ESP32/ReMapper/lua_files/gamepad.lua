-- Debug mouse (optional)
register_mouse_callback(function(buttons, dx, dy, wheel)
    -- set_mouse_raw(buttons, dx, dy, wheel)
    -- print("mouse:", buttons, dx, dy, wheel)
end)

----------------------------------------------------------
-- KEYCODES (HID usage IDs in decimal)
----------------------------------------------------------
local KEY_A     = 4    -- 'a'
local KEY_D     = 7    -- 'd'
local KEY_S     = 22   -- 's'
local KEY_W     = 26   -- 'w'
local KEY_SPACE = 44   -- Spacebar

----------------------------------------------------------
-- GAMEPAD STATE
----------------------------------------------------------
local buttons = 0      -- bitfield, but we’ll just use bit 0 for "A" button
local x = 0            -- left stick X  (-127..127)
local y = 0            -- left stick Y  (-127..127)
local rx = 0           -- right stick X (unused)
local ry = 0           -- right stick Y (unused)
local hat = 0          -- keep neutral for now

-- Track which keys are currently held
local key_w_down = false
local key_s_down = false
local key_a_down = false
local key_d_down = false
local space_down = false

----------------------------------------------------------
-- HELPER: recompute axes from key states
----------------------------------------------------------
local function update_axes()
    x = 0
    y = 0

    -- Horizontal (A/D)
    if key_a_down and not key_d_down then
        x = -127
    elseif key_d_down and not key_a_down then
        x = 127
    end

    -- Vertical (W/S) – typically up is negative in gamepad conventions
    if key_w_down and not key_s_down then
        y = -127
    elseif key_s_down and not key_w_down then
        y = 127
    end
end

local function update_buttons()
    -- Only use bit 0 as "A button" mapped from space bar
    if space_down then
        buttons = 0x01
    else
        buttons = 0x00
    end
end

----------------------------------------------------------
-- KEYBOARD CALLBACK
----------------------------------------------------------
register_keyboard_callback(function(keycode, pressed)
    -- print("Key:", keycode, pressed and "down" or "up")

    if keycode == KEY_W then
        key_w_down = pressed
    elseif keycode == KEY_S then
        key_s_down = pressed
    elseif keycode == KEY_A then
        key_a_down = pressed
    elseif keycode == KEY_D then
        key_d_down = pressed
    elseif keycode == KEY_SPACE then
        space_down = pressed
    end

    update_axes()
    update_buttons()
end)

----------------------------------------------------------
-- MAIN LOOP: continuously send gamepad state
----------------------------------------------------------
while true do
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    -- small delay so we don’t hammer the runtime / USB stack
    wait_ms(10)   -- or sleep(10) if that’s what your env provides
end
