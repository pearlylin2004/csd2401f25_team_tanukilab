local logo1 = nil
local logo2 = nil
local logo3 = nil

local timer = 0.0
local phase = 0

local FADE_IN  = 2.5
local HOLD     = 2.5
local FADE_OUT = 2.5

local SplashController = {}

function SplashController.Update(obj, dt)
    if isEditingMode() then
        logo1 = nil
        return
    end

    if logo1 == nil then
        timer = 0.0
        phase = 0
        logo1 = getGameObject("digipen")
        logo2 = getGameObject("team")
        logo3 = getGameObject("fmod")
        setRenderAlpha(logo1, 0.0)
        setRenderAlpha(logo2, 0.0)
        setRenderAlpha(logo3, 0.0)
    end

    if phase ~= 8 then
        if isKeyTriggered("SPACE") or isGamepadButtonTriggered("A") then
            setRenderAlpha(logo1, 0.0)
            setRenderAlpha(logo2, 0.0)
            setRenderAlpha(logo3, 0.0)
            phase = 8
            logo1 = nil
            loadLevel("menu_scene.json")
            return
        end
    end

    if phase == 8 then return end
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
        if timer >= FADE_OUT then phase = 6; timer = 0 end

    elseif phase == 6 then
        local a = math.min(timer / FADE_IN, 1.0)
        setRenderAlpha(logo3, a)
        if timer >= FADE_IN then phase = 7; timer = 0 end

    elseif phase == 7 then
        setRenderAlpha(logo3, 1.0)
        if timer >= HOLD then
            setRenderAlpha(logo3, 0.0)
            phase = 8
            logo1 = nil
            loadLevel("menu_scene.json")
        end
    end
end

return SplashController