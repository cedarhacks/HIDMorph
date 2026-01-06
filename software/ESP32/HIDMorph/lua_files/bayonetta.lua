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

local KEY_H      = 11    -- 'h' (grab)
local KEY_I      = 12    -- 'i' (jump Y)
local KEY_J      = 13    -- 'j' (attack A)
local KEY_K      = 14    -- 'k' (special B)
local KEY_O      = 18    -- 'o' (shield)
local KEY_U      = 24    -- 'u' (jump X)

local KEY_Q      = 20    -- 'q'  (macro: side-B forward)
local KEY_E      = 8     -- 'e'  (macro: jump → up-B)
local KEY_R      = 21    -- 'r'  (macro: ladder starter)

local KEY_ENTER  = 40    -- Return/Enter (start)

-- Arrow keys
local KEY_RIGHT  = 79
local KEY_LEFT   = 80
local KEY_DOWN   = 81
local KEY_UP     = 82

----------------------------------------------------------
-- GAMEPAD BUTTON BITS
----------------------------------------------------------
local BTN_A      = 0x01  -- attack
local BTN_B      = 0x02  -- special
local BTN_X      = 0x04  -- jump
local BTN_Y      = 0x08  -- jump 2
local BTN_L      = 0x10  -- shield L
local BTN_R      = 0x20  -- shield R
local BTN_Z      = 0x40  -- grab
local BTN_START  = 0x80  -- start

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
local x_btn_down     = false  -- jump (U)
local y_btn_down     = false  -- jump (I)
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

    -- Up is usually negative Y on gamepads
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
-- MACRO HELPERS
----------------------------------------------------------
local function save_state()
    return buttons, x, y, rx, ry, hat
end

local function restore_state(b, lx, ly, rcx, rcy, h)
    buttons = b
    x, y, rx, ry, hat = lx, ly, rcx, rcy, h
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
end

----------------------------------------------------------
-- BAYONETTA MACROS
----------------------------------------------------------
-- Q: Side-B forward (tap right + B)
local function macro_side_b_forward()
    local ob, ox, oy, orx, ory, oh = save_state()

    -- lean stick right and press B briefly
    x = STICK_VAL
    buttons = ob | BTN_B
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(80)   -- adjust for your game

    -- release special but keep original state
    restore_state(ob, ox, oy, orx, ory, oh)
end

-- E: Jump → Up-B (Witch Twist) quickly
local function macro_jump_up_b()
    local ob, ox, oy, orx, ory, oh = save_state()

    -- press jump + up
    y = -STICK_VAL
    buttons = ob | BTN_X
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(40)

    -- then press special for up-B
    buttons = buttons | BTN_B
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(70)

    -- restore
    restore_state(ob, ox, oy, orx, ory, oh)
end

-- R: Simple ladder starter: jump → side-B up → up-B
local function macro_ladder_starter()
    local ob, ox, oy, orx, ory, oh = save_state()

    -- 1) short jump
    y = -STICK_VAL
    buttons = ob | BTN_X
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(40)

    -- 2) angled up side-B (After Burner Kick up)
    --     up + towards (right). Flip sign if you want default left.
    x = STICK_VAL
    y = -STICK_VAL
    buttons = (buttons | BTN_B)
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(80)

    -- 3) then up-B (Witch Twist)
    --     keep up, remove horizontal bias for clean vertical
    x = 0
    y = -STICK_VAL
    buttons = (buttons | BTN_B)  -- press B again
    set_gamepad_raw(buttons, x, y, rx, ry, hat)
    wait_ms(80)

    -- restore original
    restore_state(ob, ox, oy, orx, ory, oh)
end

----------------------------------------------------------
-- KEYBOARD CALLBACK
----------------------------------------------------------
register_keyboard_callback(function(keycode, pressed)
    -- *** Macros (trigger on key down only) ***
    if pressed then
        if keycode == KEY_Q then
            macro_side_b_forward()
            return
        elseif keycode == KEY_E then
            macro_jump_up_b()
            return
        elseif keycode == KEY_R then
            macro_ladder_starter()
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
    wait_ms(10)
end
