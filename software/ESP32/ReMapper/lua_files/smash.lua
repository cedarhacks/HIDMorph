-- Optional mouse callback (can be removed)
register_mouse_callback(function(buttons, dx, dy, wheel)
    -- set_mouse_raw(buttons, dx, dy, wheel)
    -- print("mouse:", buttons, dx, dy, wheel)
end)

----------------------------------------------------------
-- HID KEYCODES (decimal)
----------------------------------------------------------
local KEY_A      = 4     -- 'a'
local KEY_D      = 7     -- 'd'
local KEY_S      = 22    -- 's'
local KEY_W      = 26    -- 'w'

local KEY_H      = 11    -- 'h'
local KEY_I      = 12    -- 'i'
local KEY_J      = 13    -- 'j'
local KEY_K      = 14    -- 'k'
local KEY_O      = 18    -- 'o'
local KEY_U      = 24    -- 'u'

local KEY_Q      = 20    -- 'q'  (macro: short hop)
local KEY_E      = 8     -- 'e'  (macro: short hop fair right)
local KEY_R      = 21    -- 'r'  (macro: shield grab)

local KEY_ENTER  = 40    -- Return/Enter

-- Arrow keys
local KEY_RIGHT  = 79
local KEY_LEFT   = 80
local KEY_DOWN   = 81
local KEY_UP     = 82

----------------------------------------------------------
-- GAMEPAD BUTTON BITS
----------------------------------------------------------
local BTN_A      = 0x01  -- bit 0 (attack)
local BTN_B      = 0x02  -- bit 1 (special)
local BTN_X      = 0x04  -- bit 2 (jump)
local BTN_Y      = 0x08  -- bit 3 (jump 2)
local BTN_L      = 0x10  -- bit 4 (shield)
local BTN_R      = 0x20  -- bit 5 (shield)
local BTN_Z      = 0x40  -- bit 6 (grab)
local BTN_START  = 0x80  -- bit 7 (start)

----------------------------------------------------------
-- GAMEPAD STATE
----------------------------------------------------------
local buttons = 0
local x  = 0   -- left stick X
local y  = 0   -- left stick Y
local rx = 0   -- right stick X (C-stick)
local ry = 0   -- right stick Y (C-stick)
local hat = 0  -- D-pad neutral

----------------------------------------------------------
-- KEY STATE
----------------------------------------------------------
-- Left stick (movement)
local w_down = false
local a_down = false
local s_down = false
local d_down = false

-- C-stick (right stick)
local up_down    = false
local down_down  = false
local left_down  = false
local right_down = false

-- Buttons
local a_btn_down     = false  -- attack (J)
local b_btn_down     = false  -- special (K)
local x_btn_down     = false  -- jump X (U)
local y_btn_down     = false  -- jump Y (I)
local shield_down    = false  -- shield (O)
local grab_down      = false  -- grab (H)
local start_down     = false  -- start (Enter)

----------------------------------------------------------
-- HELPERS
----------------------------------------------------------
local STICK_VAL = 127  -- full deflection

local function update_left_stick()
    x, y = 0, 0

    if a_down and not d_down then
        x = -STICK_VAL
    elseif d_down and not a_down then
        x = STICK_VAL
    end

    -- In most gamepad conventions: up is negative Y
    if w_down and not s_down then
        y = -STICK_VAL
    elseif s_down and not w_down then
        y = STICK_VAL
    end
end

local function update_right_stick()
    rx, ry = 0, 0

    if left_down and not right_down then
        rx = -STICK_VAL
    elseif right_down and not left_down then
        rx = STICK_VAL
    end

    if up_down and not down_down then
        ry = -STICK_VAL
    elseif down_down and not up_down then
        ry = STICK_VAL
    end
end

local function update_buttons()
    local b = 0

    if a_btn_down     then b = b | BTN_A           end
    if b_btn_down     then b = b | BTN_B           end
    if x_btn_down     then b = b | BTN_X           end
    if y_btn_down     then b = b | BTN_Y           end
    if shield_down    then b = b | BTN_L | BTN_R   end  -- both shields
    if grab_down      then b = b | BTN_Z           end
    if start_down     then b = b | BTN_START       end

    buttons = b
end

----------------------------------------------------------
-- MACROS
----------------------------------------------------------

-- Save & restore whole pad state
local function save_state()
    return buttons, x, y, rx, ry, hat
end

local function restore_state(b, lx, ly, rcx, rcy, h)
    buttons = b
    x, y, rx, ry, hat = lx, ly, rcx, rcy, h
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
end

-- Q: Short hop (tap jump)
local function macro_short_hop()
    print("shorttt hopp")
    local ob, ox, oy, orx, ory, oh = save_state()

    -- press jump (X)
    buttons = buttons | BTN_Y
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(40)  -- tweak for your game

    -- release jump
    buttons = ob
    set_gamepad_raw(buttons, x, y, rx, ry, hat)

    restore_state(ob, ox, oy, orx, ory, oh)
end

-- E: Short-hop forward aerial to the right (jump + move right + A)
local function macro_short_hop_fair_right()
    local ob, ox, oy, orx, ory, oh = save_state()

    -- move stick right + jump
    x = STICK_VAL
    buttons = buttons | BTN_X
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(40)

    -- then quickly press attack (A)
    buttons = buttons | BTN_A
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(60)

    -- release A + jump, keep movement as before
    buttons = ob
    set_gamepad_raw(buttons, x, y, rx, ry, hat)

    restore_state(ob, ox, oy, orx, ory, oh)
end

-- R: Shield grab (shield + grab briefly)
local function macro_shield_grab()
    local ob, ox, oy, orx, ory, oh = save_state()

    -- press shield + grab
    buttons = buttons | BTN_L | BTN_R | BTN_Z
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(80)

    -- release them
    buttons = ob
    set_gamepad_raw(buttons, x, y, rx, ry, hat)

    restore_state(ob, ox, oy, orx, ory, oh)
end

----------------------------------------------------------
-- KEYBOARD CALLBACK
----------------------------------------------------------
register_keyboard_callback(function(keycode, pressed)
    -- print("Key:", keycode, pressed and "down" or "up")

    -- --- MACRO KEYS (on key down only) ---
    if pressed then
        if keycode == KEY_Q then
            macro_short_hop()
            return
        elseif keycode == KEY_E then
            macro_short_hop_fair_right()
            return
        elseif keycode == KEY_R then
            macro_shield_grab()
            return
        end
    end

    -- Movement (left stick)
    if keycode == KEY_W then
        w_down = pressed
    elseif keycode == KEY_A then
        a_down = pressed
    elseif keycode == KEY_S then
        s_down = pressed
    elseif keycode == KEY_D then
        d_down = pressed

    -- C-stick (right stick)
    elseif keycode == KEY_UP then
        up_down = pressed
    elseif keycode == KEY_DOWN then
        down_down = pressed
    elseif keycode == KEY_LEFT then
        left_down = pressed
    elseif keycode == KEY_RIGHT then
        right_down = pressed

    -- Buttons
    elseif keycode == KEY_J then
        a_btn_down = pressed      -- attack
    elseif keycode == KEY_K then
        b_btn_down = pressed      -- special
    elseif keycode == KEY_U then
        x_btn_down = pressed      -- jump (X)
    elseif keycode == KEY_I then
        y_btn_down = pressed      -- jump (Y)
    elseif keycode == KEY_O then
        shield_down = pressed     -- shield
    elseif keycode == KEY_H then
        grab_down = pressed       -- grab
    elseif keycode == KEY_ENTER then
        start_down = pressed      -- start
    end

    update_left_stick()
    update_right_stick()
    update_buttons()
end)

----------------------------------------------------------
-- MAIN LOOP
----------------------------------------------------------
while true do
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(10)   -- tweak as needed
end
