local logo1 = nil
local logo2 = nil
local timer = 0.0
local phase = 0
local FADE_IN  = 2.0
local HOLD     = 2.5
local FADE_OUT = 2.0
local SplashController = {}
function SplashController.Update(obj, dt)
    if isEditingMode() then
        logo1 = nil
        return
    end

    -- lazy init on first frame, also resets stale state on re-simulation
    if logo1 == nil then
        timer = 0.0
        phase = 0
        logo1 = getGameObject("digipen")
        logo2 = getGameObject("team")
        setRenderAlpha(logo1, 0.0)
        setRenderAlpha(logo2, 0.0)
    end

    -- skip splash on SPACE/CROSS button on gamepad
    if phase ~= 6 then
        if isKeyTriggered("SPACE") or isGamepadButtonTriggered("A") then
            setRenderAlpha(logo1, 0.0)
            setRenderAlpha(logo2, 0.0)
            phase = 6
            logo1 = nil  -- force reset next simulation
            loadLevel("menu_scene.json")
            return
        end
    end

    if phase == 6 then return end
    timer = timer + dt

    if phase == 0 then
        local a = math.min(timer / FADE_IN, 1.0)
        setRenderAlpha(logo1, a)
        if timer >= FADE_IN then phase = 1; timer = 0 end

    elseif phase == 1 then
        setRenderAlpha(logo1, 1.0)
        if timer >= HOLD then phase = 2; timer = 0 end

    elseif phase == 2 then
        local a = 1.0 - math.min(timer / FADE_OUT, 1.0)
        setRenderAlpha(logo1, a)
        if timer >= FADE_OUT then phase = 3; timer = 0 end

    elseif phase == 3 then
        local a = math.min(timer / FADE_IN, 1.0)
        setRenderAlpha(logo2, a)
        if timer >= FADE_IN then phase = 4; timer = 0 end

    elseif phase == 4 then
        setRenderAlpha(logo2, 1.0)
        if timer >= HOLD then phase = 5; timer = 0 end

    elseif phase == 5 then
        local a = 1.0 - math.min(timer / FADE_OUT, 1.0)
        setRenderAlpha(logo2, a)
        if timer >= FADE_OUT then
            setRenderAlpha(logo2, 0.0)
            phase = 6
            logo1 = nil  -- force reset next simulation
            loadLevel("menu_scene.json")
        end
    end

end

return SplashController