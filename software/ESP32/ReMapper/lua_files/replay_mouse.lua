print("-> [LUA] Replaying mouse movements")

local filename = "mouse_record.csv"

-- Try to open recorded file
local f = io.open(filename, "r")
if not f then
    print("Could not open", filename)
    display_set_text("Replay failed\n(no file)")
    return
end

----------------------------------------------------------
-- HOMING PHASE
----------------------------------------------------------
display_set_text("Homing...")

local HOMING_STEPS = 100
local HOMING_DX = -10
local HOMING_DY = -10

for i = 1, HOMING_STEPS do
    set_mouse_pos(HOMING_DX, HOMING_DY)
    -- optional delay to avoid OS rejecting rapid input
    -- sleep(5)
    wait_ms(10)
end

print("Homing complete")

----------------------------------------------------------
-- START REPLAY
----------------------------------------------------------

display_set_text("Replaying")

-- Skip CSV header
f:read("*l")

-- Delay between replayed events (tweak)
local DELAY_MS = 5

start_time_ms = get_time_ms();

for line in f:lines() do
    local time_ms, buttons, dx, dy, wheel =
        line:match("([^,]+),([^,]+),([^,]+),([^,]+),([^,]+)")

    time_ms = tonumber(time_ms)
    buttons = tonumber(buttons)
    dx = tonumber(dx)
    dy = tonumber(dy)
    wheel = tonumber(wheel)

    -- Wait until real clock has passed recorded timestamp
    while (get_time_ms() - start_time_ms) < time_ms do
        wait_ms(1)
    end

    -- Replay relative mouse movement
    if dx ~= 0 or dy ~= 0 then
        set_mouse_raw(buttons, dx, dy, wheel)
    end

    -- if DELAY_MS > 0 then
    --     sleep(DELAY_MS)
    -- end
end

f:close()

display_set_text(ICON.OK .. "\nReplay done")
print("-> [LUA] Replay finished")
