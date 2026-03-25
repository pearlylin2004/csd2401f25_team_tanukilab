
-- load collision handlers
dofile("./assets/scripting/collision.lua")

function onSceneLoad()
    -- bullet creation
    local template = getGameObject("bullet")
    if not template then return end

    renameGameObject(template, "bullet")
    setCollisionTag(template, "bullet")
    setActive(template, false)

    for i = 1, 20 do
        local name = "bullet" .. i
        if not getGameObject(name) then
            local cloned = cloneGameObject("bullet", name)
            if cloned then
                setActive(cloned, false)
                setPosition(cloned, -1000, -1000)
                setDynPos(cloned, -1000, -1000)
                setAlive(cloned, false)
                setCollisionTag(cloned, "bullet")
            end
        end
    end
end