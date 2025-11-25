--exit_button

local exit_button = {}

--check on click
function onClick()
    print("Exit button pressed from Lua!")
    quitGame()
end

return exit_button