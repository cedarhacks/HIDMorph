-- infinite_math_test.lua
-- Simple Lua script with an infinite loop doing math operations

local t = 0
while true do
    -- Do some trigonometric math
    local x = math.sin(t) * math.cos(t / 2) + math.tan(t / 10)
    local y = math.sqrt(math.abs(x)) * math.random()
    local z = (x + y) % math.pi

    -- Print results occasionally
    if t % 1000 == 0 then
        print(string.format("t = %d | x = %.4f | y = %.4f | z = %.4f", t, x, y, z))
    end

    -- Increment time
    t = t + 1

    -- Small sleep to avoid 100% CPU
    -- Remove or adjust for faster loops
    if t % 10000 == 0 then
        os.execute("sleep 0.01")
    end
end
