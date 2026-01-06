-- Simple Lua file writing example

local filename = "output.txt"

-- Open file in write mode ("w" truncates or creates)
local file, err = io.open(filename, "w")
if not file then
    print("Error opening file:", err)
    return
end

-- Write some text
file:write("Hello from Lua!\n")
file:write("This file was created and written successfully.\n")

-- Always close the file
file:close()

print("Wrote to", filename)
