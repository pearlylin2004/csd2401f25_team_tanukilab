--player

local player = {}

local landingGrace = 0.1

local footContactFrames = { 1, 4 }

local footstepChannels = {
    "footstep1", "footstep2", "footstep3",
    "footstep4", "footstep5", "footstep6"
}

-- per-object state table (fixes hot reload nil issue)
local objState = {}
local function getS(obj)
    if not objState[obj] then
        objState[obj] = {
            coyoteTimer     = 0.0,
            wasOnGround     = false,
            jumpBufferTimer = 0.0,
            prevOnGround    = false,
            landingTimer    = 0.0,
            initialized     = false,
            lastFrame       = -1,
            lastSoundIdx    = -1,
            lastPlatformX = nil,
            lastPlatformY = nil
        }
    end
    return objState[obj]
end

-- helper func to find bullet 
local function findBullet()
    local objs = getAllObjects()
    for i = 1, #objs do
        local o = objs[i]
        local name = getObjectName(o)
        if name:sub(1,6) == "bullet" and #name > 6 then
            if name:sub(7):match("^%d+$") then
                if hasPhysics(o) and not isAlive(o) then
                    return o
                end
            end
        end
    end
    return nil
end

-- helper func to change state
local function changeState(obj, newState)
    setPlayerState(obj, newState)
    setStateTime(obj, 0.0)
end

function player.Update(obj, dt)
    if not hasPhysics(obj) or not hasTransform(obj) then return end

    local s = getS(obj)

    if not s.initialized then
        local px, py = getPosition(obj)
        setDynVelX(obj, 0)
        setDynVelY(obj, 0)
        setVelX(obj, 0)
        setVelY(obj, 0)
        s.initialized = true
        
    end

    -- coyote time update
    local onGround = isOnGround(obj)
    s.prevOnGround = s.wasOnGround

    if onGround then
        s.coyoteTimer  = getCoyoteTime(obj)
        s.wasOnGround  = true
        s.landingTimer = landingGrace
    else
        s.coyoteTimer = s.coyoteTimer - dt
        if s.coyoteTimer < 0 then s.coyoteTimer = 0 end
        s.wasOnGround = false
    end

    local canCoyoteJump = s.coyoteTimer > 0

    -- jump buffer
    if isKeyTriggered("SPACE") then
        s.jumpBufferTimer = getJumpBufferTime(obj)
    end
    if s.jumpBufferTimer > 0 then
        s.jumpBufferTimer = s.jumpBufferTimer - dt
        if s.jumpBufferTimer < 0 then s.jumpBufferTimer = 0 end
    end

    -- SPACE to jump (tap for small jump, hold for big jump)
    if s.jumpBufferTimer > 0 and canCoyoteJump and getCanMove(obj) then
        local jumpVel = getJumpForce(obj) / getDynMass(obj)
        local px, py = getPosition(obj)
        setDynPos(obj, px, py)
        setVelY(obj, jumpVel)
        setDynVelY(obj, jumpVel)
        setOnGround(obj, false)
        s.coyoteTimer     = 0
        s.jumpBufferTimer = 0
        SendInputEvent("SPACE", 0, 0)
    end

    if isKeyReleased("SPACE") then
        local vy = getVelY(obj)
        if vy > 0 then
            setVelY(obj, vy * 0.8)
            setDynVelY(obj, vy * 0.8)
        end
    end


    -- ledge climb
    if getLedgeClimb(obj) then
        local x, y = getPosition(obj)
        local targetY = getLedgeTargetY(obj)
        local diff = targetY - y
        if math.abs(diff) < 0.05 then
            setPosition(obj, x, targetY)
            setDynPos(obj, x, targetY)
            setVelY(obj, 0)
            setDynVelY(obj, 0)
            setOnGround(obj, true)
            setLedgeClimb(obj, false)
            s.coyoteTimer = getCoyoteTime(obj)
        else
            local step = getLedgeClimbSpeed(obj) * dt
            local newY = y + math.min(step, diff)
            setPosition(obj, x, newY)
            setDynPos(obj, x, newY)
            setVelY(obj, 0)
            setDynVelY(obj, 0)
            setOnGround(obj, false)
        end
    end


    -- movement (left and right)
    local targetVelX = 0.0

    if Input_isKeyHeld("A") and getCanMove(obj) then
        targetVelX = -getMoveSpeed(obj)
        setFlipX(obj, true)
        SendInputEvent("A", 0, 0)
    elseif Input_isKeyHeld("D") and getCanMove(obj) then
        targetVelX = getMoveSpeed(obj)
        setFlipX(obj, false)
        SendInputEvent("D", 0, 0)
    end


    -- flip to face mouse when not moving
    if not Input_isKeyHeld("A") and not Input_isKeyHeld("D") then
        if not isEditorOpen() or isSceneHovered() then
            local mx = isEditorOpen() and getEditorMouseWorldX() or getMouseWorldX()
            local x, y = getPosition(obj)
            setFlipX(obj, mx < x)
        end
    end


    -- damping + position
    local friction = getFriction(obj)
    applyDamping(obj, friction, targetVelX, dt)
    if not getLedgeClimb(obj) then
        updatePosition(obj, dt)
        local px, py = getPosition(obj)
    end


    -- moving platform detection
    local platform = nil
    if isOnGround(obj) then
        local allObjs = getAllObjects()
        local px, py = getPosition(obj)
        for i = 1, #allObjs do
            local o = allObjs[i]
            if getCollisionTag(o) == "platform" then
                local ox, oy = getPosition(o)
                local dx = math.abs(ox - px)
                local dy = (oy - py)
                local px2, py2 = getPosition(obj)

                if dx < 4.0 and dy > -2.0 and dy < 1.0 then
                    platform = o
                    break
                end
            end
        end
    end

    if platform then
        local ox, oy = getPosition(platform)
        if s.lastPlatformX ~= nil then
            local pdx = ox - s.lastPlatformX
            local x, y = getPosition(obj)
            setPosition(obj, x + pdx, y)
            setDynPos(obj, x + pdx, y)
        end
        s.lastPlatformX = ox
        s.lastPlatformLostTimer = 0.0  -- reset timer when on platform
    else
        -- keep lastPlatformX for a few frames after losing platform
        if not s.lastPlatformLostTimer then s.lastPlatformLostTimer = 0.0 end
        s.lastPlatformLostTimer = s.lastPlatformLostTimer + dt
        if s.lastPlatformLostTimer > 0.1 then 
            s.lastPlatformX = nil
        end
    end


    -- FSM state update
    local velY = getVelY(obj)
    local state = getPlayerState(obj)

    if state == "Idle" then
        if Input_isKeyHeld("A") or Input_isKeyHeld("D") then
            changeState(obj, "Walking")
        elseif velY > 0 then
            changeState(obj, "Jumping")
        end
    elseif state == "Walking" then
        if not Input_isKeyHeld("A") and not Input_isKeyHeld("D") then
            changeState(obj, "Idle")
        elseif velY > 0 then
            changeState(obj, "Jumping")
        elseif not isOnGround(obj) and velY < 0 and s.landingTimer <= 0 then
            changeState(obj, "Falling")
        end
    elseif state == "Jumping" then
        if velY <= 0 then
            changeState(obj, "Falling")
        end
    elseif state == "Falling" then
        if isOnGround(obj) then
            if Input_isKeyHeld("A") or Input_isKeyHeld("D") then
                changeState(obj, "Walking")
            else
                changeState(obj, "Idle")
            end
        end
    end

    setStateTime(obj, getStateTime(obj) + dt)

    -- footstep sync to animation
    if state == "Walking" then
        local frame = getAnimationFrame(obj)
        if frame ~= s.lastFrame then
            s.lastFrame = frame
            for _, contactFrame in ipairs(footContactFrames) do
                if frame == contactFrame then
                    local idx = math.random(1, #footstepChannels)
                    if #footstepChannels > 1 then
                        while idx == s.lastSoundIdx do
                            idx = math.random(1, #footstepChannels)
                        end
                    end
                    s.lastSoundIdx = idx
                    playSound(obj, footstepChannels[idx])
                    break
                end
            end
        end
    else
        s.lastFrame = -1
    end

    if s.landingTimer > 0 then
        s.landingTimer = s.landingTimer - dt
    end

    -- shoot
    if not canShoot(obj) then return end

    local shootTimer = getShootTimer(obj)
    shootTimer = shootTimer + dt
    setShootTimer(obj, shootTimer)

    if not isMouseLeftHeld() or (isEditorOpen() and not isSceneHovered()) then
        setShootTimer(obj, getFireRate(obj))
        return
    end

    -- check bullet count
    if not hasVSGlobalFloat("bullets") or getVSGlobalFloat("bullets") <= 0 then
        debugLogPlay("Out of bullets!")
        return
    end

    if shootTimer < getFireRate(obj) then return end

    local bullet = findBullet()
    if not bullet then return end

    -- find weapon child
    local spawnObj = obj
    local allObjs = getAllObjects()
    for i = 1, #allObjs do
        local candidate = allObjs[i]
        if hasWeaponComponent(candidate) and getParentName(candidate) == getObjectName(obj) then
            spawnObj = candidate
            break 
        end
    end

    local mx = isEditorOpen() and getEditorMouseWorldX() or getMouseWorldX()
    local my = isEditorOpen() and getEditorMouseWorldY() or getMouseWorldY()

    -- shoot
    playSound(bullet, "default")
    local bx, by = getPosition(spawnObj)
    local dx = mx - bx
    local dy = my - by
    local mag = math.sqrt(dx*dx + dy*dy)
    if mag > 0 then dx = dx/mag; dy = dy/mag end
    local speed = getMoveSpeed(bullet) * 3.0
    setDynVelX(bullet, dx * speed)
    setDynVelY(bullet, dy * speed)
    setDynPos(bullet, bx, by)
    setPosition(bullet, bx, by)
    setFlipX(bullet, dx < 0)
    setLifeTimer(bullet, 0.0)
    setScale(bullet, getInitialScale(bullet), getInitialScale(bullet))
    setGravityDelayTimer(bullet, getGravityDelay(bullet))
    setAlive(bullet, true)
    setOnGround(bullet, false)
    setRenderAlpha(bullet, 1.0)

    setShootTimer(obj, 0.0)
    setVSGlobalFloat("bullets", getVSGlobalFloat("bullets") - 1)
    debugLogPlay("Bullet fired!")
    SendInputEvent("LEFT_CLICK", mx, my)
end

return player