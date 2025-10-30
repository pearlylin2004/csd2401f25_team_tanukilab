-- object1.lua
print("object1.lua is loaded!")
--print("object1.lua is loaded!") --for testing

function Update(obj, deltaTime)
    print("lua is running!!!")
    --print("lua is running!!!") --for testing
    --for going up
    if Input_isKeyHeld("W") then
        print("input W")
    end
    --for going down
    if Input_isKeyHeld("S") then
        print("input S")
    end
    --for going left
    if Input_isKeyHeld("A") then
        print("input A")
    end
    --for going right
    if Input_isKeyHeld("D") then
        print("input D")
    end