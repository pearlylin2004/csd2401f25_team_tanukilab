local buoyancy = {}

-- per-object state
local objState = {}
local function getS(obj)
    if not objState[obj] then
        objState[obj] = { vy = 0.0, time = 0.0, playerWasOn = false, landCooldown = 0.0 }
    end
    return objState[obj]
end

function buoyancy.Update(obj, dt)
    if not hasPhysics(obj) or not hasTransform(obj) then return end
    if isEditingMode() or isPaused() then return end

    local allObjs = getAllObjects()
    local inWater = false

    for i = 1, #allObjs do
        local waterObj = allObjs[i]
        if not hasWaterBody(waterObj) then goto continue end

        local waterW = getWaterWidth(waterObj)
        local waterH = getWaterHeight(waterObj)
        local wx, wy = getPosition(waterObj)
        local waterMinX = wx - waterW * 0.5
        local waterMaxX = wx + waterW * 0.5
        local waterMinY = wy - waterH * 0.5
        local waterMaxY = wy + waterH * 0.5

        local ox, oy = getPosition(obj)
        if ox < waterMinX or ox > waterMaxX then goto continue end
        if oy < waterMinY or oy > waterMaxY then goto continue end

        inWater = true

        if getBuoyancy(obj) then
            local s = getS(obj)
            local scaleY = getScaleY(obj)
            local waterSurfaceY = wy + waterH * 0.5

            -- follow water rise instantly
            local lastRise = getWaterLastFrameRise(waterObj)
            if lastRise > 0.0 then
                oy = oy + lastRise
                --s.vy = s.vy + lastRise / dt  -- give upward impulse
            end

            -- physics-based values
            local mass = getDynMass(obj)
            local buoyancyForce = getWaterBuoyancyForce(waterObj)
            local drag = getWaterDrag(waterObj)

            -- rest position based on mass
            local floatOffset = 0.5 * (1.0 / mass)
            local restY = waterSurfaceY - scaleY * 0.5 + floatOffset

            -- spring force based on water buoyancy and mass
            local springK = (buoyancyForce / 50.0) * 12.0 * (1.0 / mass)
            local damping = 1.5 + (1.0 - drag) * 5.0

            -- constant bob scaled by mass
            local bobFreq = 1.5
            local bobAmp  = 0.008 * (1.0 / mass)

            -- player landing impulse
            local allObjs2 = getAllObjects()
            for j = 1, #allObjs2 do
                local other = allObjs2[j]
                if getCollisionTag(other) == "player" then
                    local px, py = getPosition(other)
                    local platformTop = oy + scaleY * 0.5

                    local onPlatform = math.abs(px - ox) < getScaleX(obj) * 0.5 and
                                    math.abs(py - platformTop) < 1.5

                    s.landCooldown = math.max(0.0, s.landCooldown - dt)

                    if onPlatform then
                        -- continuous downward pressure while player is on
                        s.vy = s.vy - 1.5 * dt
                        -- first landing impulse
                        if not s.playerWasOn and s.landCooldown <= 0.0 then
                            s.vy = s.vy - 1.5
                            s.landCooldown = 0.5
                        end
                    end
                    s.playerWasOn = onPlatform
                end
            end

            -- spring force
            local springK = 12.0   -- stiffness
            local damping = 1.5    -- damping (higher = less oscillation)
            local displacement = restY - oy
            local springAccel = displacement * springK
            s.vy = s.vy + springAccel * dt
            s.vy = s.vy - s.vy * damping * dt

            -- constant bob
            local bobFreq = 1.5
            local bobAmp  = 0.008
            s.vy = s.vy + math.sin(s.time * bobFreq * math.pi * 2) * bobAmp
            s.time = s.time + dt

            -- clamp velocity
            s.vy = math.max(-8.0, math.min(s.vy, 8.0))

            -- integrate position
            local newY = oy + s.vy * dt

            setPosition(obj, ox, newY)
            setDynPos(obj, ox, newY)
            -- zero out physics velocity so collision system doesnt fight us
            setVelY(obj, 0)
            setDynVelY(obj, 0)

            break
        end

        ::continue::
    end

    setInWater(obj, inWater)
end

return buoyancy