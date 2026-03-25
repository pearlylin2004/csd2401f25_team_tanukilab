-- WaterBound collision handlers

function onCollision(a, b)
    local tagA = getCollisionTag(a)
    local tagB = getCollisionTag(b)

    -- floatingObject + water
    if (tagA == "floatingObject" and hasWaterBody(b)) or
       (tagB == "floatingObject" and hasWaterBody(a)) then
        local floater = tagA == "floatingObject" and a or b
        setInWater(floater, true)
        return false
    end

    -- bullet + bullet (water particle merge)
    -- leave to engine for now
    if tagA == "bullet" and tagB == "bullet" then
        return false
    end

    -- bullet + water
    local bulletIsA = tagA == "bullet" and tagB == "water"
    local bulletIsB = tagB == "bullet" and tagA == "water"
    if bulletIsA or bulletIsB then
        local bullet = bulletIsA and a or b
        local water  = bulletIsA and b or a
        -- deactivate bullet
        local p = hasPhysics(bullet)
        if p then
            if not isOriginalStateSet(bullet) then
                setOriginalPos(bullet)
            end
            local ox, oy = getPosition(bullet)
            setPosition(bullet, -1000, -1000)
            setDynPos(bullet, -1000, -1000)
            setDynVelX(bullet, 0)
            setDynVelY(bullet, 0)
            clearDynForces(bullet)
            setLifeTimer(bullet, 0)
            setAlive(bullet, false)
            setRenderAlpha(bullet, 0)
        end
        addWaterHeight(water)
        return true
    end

    return false
end

function onCollisionResponse(obj, tag, otherTag, nx, ny, isStatic, penetration)
    -- bullet bounce
    if tag == "bullet" then
        if nx ~= 0 then return true end
        if ny ~= 0 then
            local vx = getDynVelX(obj)
            local dir = 1.0
            if vx < 0 then dir = -1.0 end
            setDynVelX(obj, 12.0 * dir)
            setDynVelY(obj, 0)
            setOnGround(obj, true)
            return true
        end
    end

    -- floatingObject impact
    if tag == "floatingObject" and otherTag ~= "bullet" then
        if ny < 0 and not isStatic then
            local approxVel = -math.abs(penetration) * 60.0
            local vy = getVelY(obj)
            vy = vy + approxVel * 0.7
            vy = math.max(vy, -15.0)
            setVelY(obj, vy)
            setDynVelY(obj, vy)
        end
        return false
    end

    return false
end

function getAudioListenerPos()
    local objs = getAllObjects()
    for i = 1, #objs do
        local o = objs[i]
        if getCollisionTag(o) == "player" then
            return getPosition(o)
        end
    end
    return 0, 0
end