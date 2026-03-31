
--   CTRL + N : skip to next level/scene
--   CTRL + E : skip straight to ending cutscene (or menu if its alr at the ending cutscene))
--   CTRL + R : restart current level

local cheat = {}

-- level order
local levelOrder = {
    "Opening_Cutscene.json",
    "Game_Level_Tutorial.json",
    "Game_Level_1.json",
    "Game_Level_2.json",
    "Game_Level_3.json",
    "Game_Level_5.json",
    "Ending_Cutscene.json",
}

local function getNextLevel(current)
    for i = 1, #levelOrder - 1 do
        if levelOrder[i] == current then
            return levelOrder[i + 1]
        end
    end

    -- if alr at the end cutscene, loop back to menu
    return "menu_scene.json"
end

function cheat.Update(obj, dt)
    if isEditingMode() or isPaused() or isEditorOpen() then return end

    local current = getCurrentScene()

    -- CTRL + N: skip to next
    if isComboKeyTriggered("N", "CTRL") then
        loadLevel(getNextLevel(current))
    end

    -- CTRL + E: skip to ending (or menu if alr end)
    if isComboKeyTriggered("E", "CTRL") then
        if current == "Ending_Cutscene.json" then
            loadLevel("menu_scene.json")
        else
            loadLevel("Ending_Cutscene.json")
        end
    end

    
    -- CTRL + R: restart current level
    if isComboKeyTriggered("R", "CTRL") then
        loadLevel(current)
    end
end

return cheat