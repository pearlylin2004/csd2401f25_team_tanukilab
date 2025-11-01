-- object1.lua
--print("object1.lua is loaded!") --for testing

Object1 = {}
function Object1.Update(obj, deltaTime)
    --print("lua is running!!!") --for testing
    local speed = 5
    local x, y = getPosition(obj)
    --for going up
    if Input_isKeyHeld("W") then
        y = y + speed * deltaTime
        setPosition(obj, x, y)
        print(string.format("input W, x: %.2f, y: %.2f", x, y))
    end
    --for going down
    if Input_isKeyHeld("S") then
        y = y - speed * deltaTime
        setPosition(obj, x, y)
        print(string.format("input S, x: %.2f, y: %.2f", x, y))
    end
    --for going left
    if Input_isKeyHeld("A") then
        x = x - speed * deltaTime
        setPosition(obj, x, y)
        print(string.format("input A, x: %.2f, y: %.2f", x, y))
    end
    --for going right
    if Input_isKeyHeld("D") then
        x = x + speed * deltaTime
        setPosition(obj, x, y)
        print(string.format("input D, x: %.2f, y: %.2f", x, y))
    end
end