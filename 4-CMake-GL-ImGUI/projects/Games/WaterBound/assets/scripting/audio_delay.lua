local audioDelay = {}

local timer = 0
local bgmPlayed = false

function audioDelay.Update(obj, dt)
    if isEditingMode() or isPaused() then return end
    if bgmPlayed then return end

    timer = timer + dt
    --print("saghfksdhjfkl")
    --if timer >= 1.0 then
        requestBGM("opening_bgm")
        --bgmPlayed = true
    --end
end

return audioDelay