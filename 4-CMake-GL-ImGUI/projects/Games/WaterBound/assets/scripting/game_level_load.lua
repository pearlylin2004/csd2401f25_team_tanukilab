
dofile("./assets/scripting/common.lua")

function onSceneLoad()
    requestBGM("BGM")

    -- bullet pool setup
    local template = getGameObject("bullet")
    if not template then return end
    setCollisionTag(template, "bullet")
    setActive(template, false)
    for i = 1, 20 do
        local name = "bullet" .. i
        if not getGameObject(name) then
            local cloned = cloneGameObject("bullet", name)
            if cloned then
                setExcludeFromSave(cloned, true) --do not serialize cloned bullet obj

                setActive(cloned, false)
                setPosition(cloned, -1000, -1000)
                setDynPos(cloned, -1000, -1000)
                setAlive(cloned, false)
                setCollisionTag(cloned, "bullet")
            end
        end
    end
end

return {}