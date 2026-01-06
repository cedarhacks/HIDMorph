-- test.lua
-- A minimal Lua script to verify embedded Lua is working

print("Hello from Lua on ESP32!")

-- Define a function
function add(a, b)
    return a + b
end

-- Call it
local x = 10
local y = 32
print("The sum is:", add(x, y))

-- Demonstrate looping and table usage
local nums = {1, 2, 3, 4, 5}
local total = 0
for i, n in ipairs(nums) do
    total = total + n
end
print("Sum of table:", total)

-- Return something to C if needed
return total
