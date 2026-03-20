--bullet

local bullet = {}
 
function bullet.Update(obj, dt)
    if not hasPhysics(obj) or not hasTransform(obj) then return end
 
    -- only process numbered bullets
    local name = getObjectName(obj)
    if name:sub(1,6) ~= "bullet" or #name <= 6 then return end
    if not name:sub(7):match("^%d+$") then return end
 
    if not isAlive(obj) then
        -- keep dead bullets parked offscreen
        local x, y = getPosition(obj)
        if y > -100 or x > -100 then
            setPosition(obj, -1000, -1000)
            setDynPos(obj, -1000, -1000)
            setDynVelX(obj, 0)
            setDynVelY(obj, 0)
        end
        return
    end
 
    -- update forces
    updateForces(obj, dt)
 
    local gdt = getGravityDelayTimer(obj)
 
    if gdt > 0 then
        local fraction = 1.0 - (gdt / getGravityDelay(obj))
        local vy = getDynVelY(obj)
        vy = vy + (-15.8 * fraction * dt)
        setDynVelY(obj, vy)
        setGravityDelayTimer(obj, gdt - dt)
 
        local vx = getDynVelX(obj)
        local px = getDynPosX(obj) + vx * dt
        local py = getDynPosY(obj) + vy * dt
        setDynPos(obj, px, py)
        setPosition(obj, px, py)
    else
        if not isOnGround(obj) then
            local vx = getDynVelX(obj)
            local vy = getDynVelY(obj)
            local lt = getLifeTimer(obj)
 
            local flowDir = 1.0
            if vx < 0 then flowDir = -1.0 end
            local flowStrength = 5.0 * math.exp(-lt * 0.6)
            vx = vx + flowStrength * flowDir * dt
 
            vy = vy + (-15.8 * dt)
 
            -- turbulence decays
            local turbulence = math.sin(lt * 8.0) * 2.0 * math.exp(-lt)
            vy = vy + turbulence * dt
 
            -- quadratic drag
            local speed = math.sqrt(vx*vx + vy*vy)
            if speed > 0.0001 then
                local drag = 0.5 * speed * speed
                vx = vx - (vx / speed) * drag * dt
                vy = vy - (vy / speed) * drag * dt
            end
 
            setDynVelX(obj, vx)
            setDynVelY(obj, vy)
 
            -- integrate position
            local px = getDynPosX(obj) + vx * dt
            local py = getDynPosY(obj) + vy * dt
            setDynPos(obj, px, py)
            setPosition(obj, px, py)
        end
    end
 
    -- update life timer
    local lt = getLifeTimer(obj) + dt
    setLifeTimer(obj, lt)
 
    -- shrink and fade over lifetime
    if lt > 2.0 then
        local shrinkTimer = lt - 2.0
        local lifePercent = shrinkTimer / getMaxLifetime(obj)
        local initScale = getInitialScale(obj)
        local newScale = math.max(initScale * (1.0 - lifePercent), getMinScale(obj))
        setScale(obj, newScale, newScale)
        setColliderSize(obj, newScale, newScale)
    end
 
    -- deactivate if lifetime exceeded or fell too far
    local py = getDynPosY(obj)
    if lt >= getMaxLifetime(obj) or py < -20.0 then
        -- deactivate bullet
        setAlive(obj, false)
        setRenderAlpha(obj, 0.0)
        setDynVelX(obj, 0)
        setDynVelY(obj, 0)
        setLifeTimer(obj, 0.0)
        setPosition(obj, -1000, -1000)
        setDynPos(obj, -1000, -1000)
    end
end
 
return bullet