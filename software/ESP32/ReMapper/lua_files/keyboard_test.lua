register_keyboard_callback(function(keycode, pressed)
    print("Key:", keycode, pressed and "down" or "up")

    if keycode == 26 then
        set_mouse_pos(0, 15)
    elseif keycode == 22 then
        set_mouse_pos(0, -15)
    end

end)

while true do
    -- add a small delay or yield if your runtime supports it
    -- e.g., sleep(10)
end
