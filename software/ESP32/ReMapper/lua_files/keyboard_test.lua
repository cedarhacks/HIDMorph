-- fill these in with your actual keycodes for W/A/S/D
local KEY_W = 26
local KEY_S = 22
local KEY_A = 4
local KEY_D = 7

-- track which movement keys are currently held
local key_state = {
    w = false,
    a = false,
    s = false,
    d = false
}

register_keyboard_callback(function(keycode, pressed)
    -- print("Key:", keycode, pressed and "down" or "up")

    if keycode == KEY_W then
        key_state.w = pressed
    elseif keycode == KEY_S then
        key_state.s = pressed
    elseif keycode == KEY_A then
        key_state.a = pressed
    elseif keycode == KEY_D then
        key_state.d = pressed
    end
end)

local step = 10
while true do
    local dx, dy = 0, 0


    if key_state.w then
        dy = dy - step   -- up
    end
    if key_state.s then
        dy = dy + step   -- down
    end
    if key_state.a then
        dx = dx - step   -- left
    end
    if key_state.d then
        dx = dx + step   -- right
    end

    if dx ~= 0 or dy ~= 0 then
        -- assuming set_mouse_pos sends a *relative* movement
        set_mouse_pos(dx, dy)
    end
end
