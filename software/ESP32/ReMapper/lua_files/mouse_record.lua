-- Simple state machine
local STATE_HOMING    = 1
local STATE_WAIT_MOVE = 2
local STATE_RECORDING = 3

local state           = STATE_HOMING
local log_file        = nil

-- How long to keep "homing" (tweak as needed)
local HOMING_STEPS    = 200
local HOMING_STEP_DX  = -15
local HOMING_STEP_DY  = -15

-- Mouse callback: used both for detecting the first move,
-- and for recording all subsequent moves.
register_mouse_callback(function(buttons, dx, dy, wheel)

    set_mouse_raw(buttons, dx, dy, wheel);

    if state == STATE_WAIT_MOVE then
        if dx ~= 0 or dy ~= 0 or wheel ~= 0 or buttons ~= 0 then
            log_file = assert(io.open("mouse_record.csv", "w"))
            log_file:write("buttons,dx,dy,wheel\n")
            log_file:write(string.format("%d,%d,%d,%d\n", buttons, dx, dy, wheel))
            log_file:flush()

            state = STATE_RECORDING
            display_set_text(ICON.STOP .. "\nRecording...\n")
            print("Recording started")
        end
    end

    if state == STATE_RECORDING and log_file then
        log_file:write(string.format("%d,%d,%d,%d\n", buttons, dx, dy, wheel))
        log_file:flush()
    end
end)

----------------------------------------------------------
-- Main script
----------------------------------------------------------

display_set_text("Homing Now")

local i = 0
while i < HOMING_STEPS do
    set_mouse_pos(HOMING_STEP_DX, HOMING_STEP_DY)
    i = i + 1
    wait_ms(10)

end

-- Now wait for user to move the mouse
state = STATE_WAIT_MOVE
display_set_text(ICON.PAUSE .. "\nMove mouse to start\n")
print("Homing done, waiting for movement...")

while true do
end
