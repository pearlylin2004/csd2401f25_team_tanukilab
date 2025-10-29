-- object1.lua
print("object1.lua is loaded!")

function Update(obj, deltaTime)
    print("lua is running!!!")
    if Input_isKeyHeld("W") then
        print("input W")
    end
    if Input_isKeyHeld("S") then
        print("input S")
    end
    if Input_isKeyHeld("A") then
        print("input A")
    end
    if Input_isKeyHeld("D") then
        print("input D")
    end
end