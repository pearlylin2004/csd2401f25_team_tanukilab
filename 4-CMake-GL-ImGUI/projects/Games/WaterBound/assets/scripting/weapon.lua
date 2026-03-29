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
    if isEditingMode() then return end

    local s = getS(obj)

    -- initialise base local offset once
    if not isWeaponOffsetInitialised(obj) then
        setWeaponBaseOffsetX(obj, getLocalOffsetX(obj))
        setWeaponOffsetInitialised(obj, true)
    end

    -- check owner facing direction
    local ownerFacingLeft = false
    local parent = nil
    local parentName = getParentName(obj)
    if parentName ~= "" then
        parent = getGameObject(parentName)
        if parent and hasTransform(parent) then
            ownerFacingLeft = getFlipX(parent)
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
    if parent and hasTransform(parent) then
        local ox, oy = getPosition(parent)
        local parentRot = getRotation(parent)
        local rad = math.rad(parentRot)
        local cosA = math.cos(rad)
        local sinA = math.sin(rad)
        local lox = getLocalOffsetX(obj)
        local loy = getLocalOffsetY(obj)
        local rotX = lox * cosA - loy * sinA
        local rotY = lox * sinA + loy * cosA
        setPosition(obj, ox + rotX, oy + rotY)
    end

    -- aim at mouse
    local wx, wy = getPosition(obj)
    local rightStickUsed = isGamepadConnected() and
        (math.abs(getGamepadAxisRX()) > 0.15 or math.abs(getGamepadAxisRY()) > 0.15)
    local shouldAim = not isEditorOpen() or isSceneHovered() or rightStickUsed

    if shouldAim then
        local aimOrigin = parent or obj
        local mx = isEditorOpen() and getEditorMouseWorldX(aimOrigin) or getMouseWorldX(aimOrigin)
        local my = isEditorOpen() and getEditorMouseWorldY(aimOrigin) or getMouseWorldY(aimOrigin)
        local dx = mx - wx
        local dy = my - wy
        local aimRad

        if ownerFacingLeft then
            aimRad = math.atan(-dy, -dx)
        else
            aimRad = math.atan(dy, dx)
        end

        local aimDeg = math.deg(aimRad)
        setRotation(obj, aimDeg)
        setWeaponCurrentAngle(obj, aimDeg)
    end
    setFlipX(obj, ownerFacingLeft)

end

return weapon