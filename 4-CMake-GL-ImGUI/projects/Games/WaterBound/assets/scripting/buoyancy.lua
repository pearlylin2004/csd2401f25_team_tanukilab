-- buoyancy

local buoyancy = {}

function buoyancy.Update(obj, dt)
    if not hasPhysics(obj) or not hasTransform(obj) then return end
    if isEditingMode() or isPaused() then
        setVelY(obj, 0)
        setVelX(obj, 0)
        setDynVelY(obj, 0)
        setDynVelX(obj, 0)
        return
    end

    -- find all water bodies
    local allObjs = getAllObjects()
    local inWater = false

    for i = 1, #allObjs do
        local waterObj = allObjs[i]
        if not hasWaterBody(waterObj) then goto continue end

        local waterW    = getWaterWidth(waterObj)
        local waterH    = getWaterHeight(waterObj)
        local wx, wy    = getPosition(waterObj)
        local waterMinX = wx - waterW * 0.5
        local waterMaxX = wx + waterW * 0.5
        local waterMinY = wy - waterH * 0.5
        local waterMaxY = wy + waterH * 0.5

        local ox, oy = getPosition(obj)
        local inside = ox >= waterMinX and ox <= waterMaxX and
                       oy >= waterMinY and oy <= waterMaxY

        if not inside and not inWater then goto continue end

        inWater = true

        local waterSurfaceY = wy + waterH * 0.5
        local lastRise = getWaterLastFrameRise(waterObj)

        if lastRise > 0.0 then
            -- water rose this frame, push object up with it
            local _, oy2 = getPosition(obj)
            setPosition(obj, ox, oy2 + lastRise)
            setDynPos(obj, ox, oy2 + lastRise)
            setVelY(obj, 0)
            setDynVelY(obj, 0)
            break
        end

        local depth = waterSurfaceY - oy
        local scaleY = getScaleY(obj)
        local halfH = scaleY * 0.5
        local submerged = math.max(0, math.min(depth / (halfH * 2.0 + 0.001), 1.0))

        local mass = getDynMass(obj)
        local buoyancyAccel = (getWaterBuoyancyForce(waterObj) / mass) * submerged * 0.016 * 16.0

        local vy = getVelY(obj)
        vy = vy + buoyancyAccel
        vy = math.max(getMaxFallSpeed(obj), math.min(vy, 8.0))

        local drag = getWaterDrag(waterObj)
        local vertDrag = drag * 0.999 + (drag - drag * 0.999) * submerged
        vy = vy * vertDrag

        local vx = getVelX(obj)
        vx = vx * drag

        setVelY(obj, vy)
        setDynVelY(obj, vy)
        setVelX(obj, vx)
        setDynVelX(obj, vx)

        break

        ::continue::
    end

    setInWater(obj, inWater)
end

return buoyancy