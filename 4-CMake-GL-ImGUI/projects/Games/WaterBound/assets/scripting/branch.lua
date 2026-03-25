local branch = {}

function branch.Update(obj, dt)
    if not hasPhysics(obj) or not hasTransform(obj) then return end
    if isEditingMode() or isPaused() then return end
    if not getSinlePersonalBool(obj) then return end

    setPhysicsFlag(obj, false)
    setAutoUpdate(obj, false)

    if not getSinleFlag(obj) then return end

    local ax = getAccelX(obj)
    local ay = getAccelY(obj)

    if isInWater(obj) then
        ay = getWaterBounce(obj)
    else
        ay = 0
    end

    if isInWater(obj) and getHitground(obj) == false then
        -- onEnterWater equivalent — dampen velY
        local vy = getVelY(obj)
        vy = vy * 0.5
        setVelY(obj, vy)
        setDynVelY(obj, vy)
    end

    local gravity = getSinleGravity(obj)
    local vx = getVelX(obj) + ax * dt
    local vy = getVelY(obj) + (ay + gravity) * dt

    if getHitground(obj) then
        vy = 0
        setHitground(obj, false)
    end

    setVelX(obj, vx)
    setVelY(obj, vy)
    setDynVelX(obj, vx)
    setDynVelY(obj, vy)
    updatePosition(obj, dt)
end

return branch