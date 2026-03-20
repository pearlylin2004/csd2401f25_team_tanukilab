-- weapon

local weapon = {}

local objState = {}
local function getS(obj)
    if not objState[obj] then
        objState[obj] = {
            initialized = false,
        }
    end
    return objState[obj]
end

function weapon.Update(obj, dt)

    if not hasTransform(obj) or not getAimAtMouse(obj) then return end

    local s = getS(obj)

    -- initialise base local offset once
    if not isWeaponOffsetInitialised(obj) then
        setWeaponBaseOffsetX(obj, getLocalOffsetX(obj))
        setWeaponOffsetInitialised(obj, true)
    end

    -- check owner facing direction
    local ownerFacingLeft = false
    local parentName = getParentName(obj)
    if parentName ~= "" then
        local owner = getGameObject(parentName)
        if owner and hasTransform(owner) then
            ownerFacingLeft = getFlipX(owner)
        end
    end

    -- flip local offset based on owner direction
    local baseOffset = getWeaponBaseOffsetX(obj)
    if ownerFacingLeft then
        setLocalOffsetX(obj, -math.abs(baseOffset))
    else
        setLocalOffsetX(obj, math.abs(baseOffset))
    end

    -- update world position based on parent
    if parentName ~= "" then
        local owner = getGameObject(parentName)
        if owner and hasTransform(owner) then
            local ox, oy = getPosition(owner)
            local parentRot = getRotation(owner)
            local rad = math.rad(parentRot)
            local cosA = math.cos(rad)
            local sinA = math.sin(rad)
            local lox = getLocalOffsetX(obj)
            local loy = getLocalOffsetY(obj)
            local rotX = lox * cosA - loy * sinA
            local rotY = lox * sinA + loy * cosA
            setPosition(obj, ox + rotX, oy + rotY)
        end
    end

    -- aim at mouse
    local wx, wy = getPosition(obj)
    local mx = getMouseWorldX()
    local my = getMouseWorldY()
    local dx = mx - wx
    local dy = my - wy

    local aimRad
    if ownerFacingLeft then
        aimRad = math.atan(-dy, -dx)
    else
        aimRad = math.atan(dy, dx)
    end

    setFlipX(obj, ownerFacingLeft)
    local aimDeg = math.deg(aimRad)
    setRotation(obj, aimDeg)
    setWeaponCurrentAngle(obj, aimDeg)
end

return weapon