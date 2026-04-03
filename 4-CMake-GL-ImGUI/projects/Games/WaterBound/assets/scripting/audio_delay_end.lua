local audioDelayEnd = {}

local timer = 0
local bgmPlayed = false

function audioDelayEnd.Update(obj, dt)
    if isEditingMode() or isPaused() then return end
    if bgmPlayed then return end

    timer = timer + dt
    --print("saghfksdhjfkl")
    if timer >= 1.0 then
        requestBGM("endscene_bgm")
        bgmPlayed = true
        --print("fk u y u play so fast")
    end
end

return audioDelayEnd