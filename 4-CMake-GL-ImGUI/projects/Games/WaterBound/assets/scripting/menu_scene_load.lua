
dofile("./assets/scripting/common.lua")

function onSceneLoad()
    setLowPassFilter(false)
    requestBGM("main_menu_bgm")
end

return {}