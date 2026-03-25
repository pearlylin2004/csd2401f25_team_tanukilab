# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.20)
project(TanukiEngine)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ImportDependencies)
importDependencies()

add_definitions(-DGLEW_STATIC)
add_definitions(-DUSE_CSD3151_AUTOMATION=0)

set(PROJECTS_DIR "${CMAKE_CURRENT_LIST_DIR}/projects")
set(ENGINE_DIR   "${PROJECTS_DIR}/TanukiEngine")
set(EDITOR_DIR   "${PROJECTS_DIR}/TanukiEditor")
set(GAMES_DIR    "${PROJECTS_DIR}/Games")
set(FMOD_LIB_DIR "${ENGINE_DIR}/lib")
set(FMOD_DIR     "${ENGINE_DIR}/lib/FMOD")

set(TANUKI_THIRD_PARTY
    glfw
    libglew_static
    imgui
    ImGuizmo
    stb
    lua
    rapidjson
    freetype
)

# ======================= [1] TANUKI ENGINE (Static Lib) ================

file(GLOB_RECURSE ENGINE_SOURCES
    "${ENGINE_DIR}/src/*.cpp"
    "${ENGINE_DIR}/include/*.h"
    "${ENGINE_DIR}/include/*.hpp"
)

##file(GLOB_RECURSE ENGINE_EDITOR_HEADERS "${ENGINE_DIR}/include/Editor/*.h")
##file(GLOB_RECURSE ENGINE_EDITOR_SOURCES "${ENGINE_DIR}/src/Editor/*.cpp")
##source_group("Header Files\\Editor" FILES ${ENGINE_EDITOR_HEADERS})
##source_group("Source Files\\Editor" FILES ${ENGINE_EDITOR_SOURCES})

add_library(TanukiEngine STATIC ${ENGINE_SOURCES})

file(GLOB_RECURSE ENGINE_CPP "${ENGINE_DIR}/src/*.cpp")
file(GLOB_RECURSE ENGINE_HDR "${ENGINE_DIR}/include/*.h" "${ENGINE_DIR}/include/*.hpp")
source_group(TREE "${ENGINE_DIR}/src"     PREFIX "Source Files" FILES ${ENGINE_CPP})
source_group(TREE "${ENGINE_DIR}/include" PREFIX "Header Files" FILES ${ENGINE_HDR})

set_property(TARGET TanukiEngine PROPERTY CXX_STANDARD 20)

target_include_directories(TanukiEngine PUBLIC
    "${ENGINE_DIR}/include"
    "${ENGINE_DIR}/include/Core"
    "${ENGINE_DIR}/include/Editor"
    "${ENGINE_DIR}/include/Logic"
    "${ENGINE_DIR}/include/Support"
    "${ENGINE_DIR}/include/Systems"
    "${FMOD_DIR}"
    "${lua_SOURCE_DIR}"
)

target_link_directories(TanukiEngine PUBLIC "${FMOD_LIB_DIR}")

target_link_libraries(TanukiEngine PUBLIC
    ${TANUKI_THIRD_PARTY}
    fmod_vc.lib
    fmodL_vc.lib
)

# Path macros — PUBLIC so engine, editor and games all get them
target_compile_definitions(TanukiEngine PUBLIC
    RUNTIME_ASSETS_DIR_R="./assets"
    RUNTIME_SHADER_DIR_R="./assets/shaders"
    RUNTIME_SCENE_DIR_R="./assets/Scene"
    RUNTIME_PREFAB_DIR_R="./assets/Prefab"
    RUNTIME_AUDIO_DIR_R="./assets/Audio"
    RUNTIME_SCRIPT_DIR_R="./assets/scripting"
    RUNTIME_RES_DIR_R="./assets/resources"
)

if(MSVC)
    target_compile_options(TanukiEngine PRIVATE /W4 /WX- /FS)
else()
    target_compile_options(TanukiEngine PRIVATE -Wall)
endif()

# Helper — copies FMOD DLLs next to any exe that links TanukiEngine
macro(copy_fmod_dlls TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${FMOD_LIB_DIR}/fmod.dll"  "$<TARGET_FILE_DIR:${TARGET_NAME}>/fmod.dll"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${FMOD_LIB_DIR}/fmodL.dll" "$<TARGET_FILE_DIR:${TARGET_NAME}>/fmodL.dll"
        COMMENT "Copying FMOD DLLs to ${TARGET_NAME} output"
    )
endmacro()

# Helper — copies engine assets (shaders etc.) next to any exe
macro(copy_engine_assets TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ENGINE_DIR}/assets" "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
        COMMENT "Copying engine assets to ${TARGET_NAME} output"
    )
endmacro()

# ======================= [2] TANUKI EDITOR (Executable) ================

file(GLOB_RECURSE EDITOR_SOURCES
    "${EDITOR_DIR}/src/*.cpp"
    "${EDITOR_DIR}/include/*.h"
    "${EDITOR_DIR}/include/*.hpp"
)

add_executable(TanukiEditor ${EDITOR_SOURCES})

file(GLOB_RECURSE EDITOR_CPP "${EDITOR_DIR}/src/*.cpp")
file(GLOB_RECURSE EDITOR_HDR "${EDITOR_DIR}/include/*.h" "${EDITOR_DIR}/include/*.hpp")
source_group(TREE "${EDITOR_DIR}/src"     PREFIX "Source Files" FILES ${EDITOR_CPP})
source_group(TREE "${EDITOR_DIR}/include" PREFIX "Header Files" FILES ${EDITOR_HDR})

set_property(TARGET TanukiEditor PROPERTY CXX_STANDARD 20)

target_include_directories(TanukiEditor PRIVATE
    "${EDITOR_DIR}/include"
)

target_link_libraries(TanukiEditor PRIVATE TanukiEngine)

copy_fmod_dlls(TanukiEditor)
copy_engine_assets(TanukiEditor)

target_compile_definitions(TanukiEditor PRIVATE
    TANUKI_GAMES_ROOT_R="${GAMES_DIR}"
    TANUKI_ENGINE_ASSETS_R="${ENGINE_DIR}/assets"
)

if(MSVC)
    set_target_properties(TanukiEditor PROPERTIES
        LINK_FLAGS_DEBUG   "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:TanukiEditor>"
    )
endif()

# ======================= [3] GAMES (Auto-discovered) ===================
# Every subfolder under projects/Games/ becomes a game executable.
# To add a new game: create projects/Games/NewGame/ with src/ include/ assets/
# No CMake edits needed — it gets picked up automatically.

file(GLOB game_candidates
    RELATIVE "${GAMES_DIR}"
    "${GAMES_DIR}/*"
)

foreach(game ${game_candidates})
    set(GAME_DIR "${GAMES_DIR}/${game}")

    if(NOT IS_DIRECTORY "${GAME_DIR}")
        continue()
    endif()

    message(STATUS "Configuring game: ${game}")

    file(GLOB_RECURSE ${game}_sources
        "${GAME_DIR}/src/*.cpp"
        "${GAME_DIR}/include/*.h"
        "${GAME_DIR}/include/*.hpp"
    )

    add_executable(${game} ${${game}_sources})

    file(GLOB_RECURSE ${game}_cpp "${GAME_DIR}/src/*.cpp")
    file(GLOB_RECURSE ${game}_hdr "${GAME_DIR}/include/*.h" "${GAME_DIR}/include/*.hpp")
    source_group(TREE "${GAME_DIR}/src"     PREFIX "Source Files" FILES ${${game}_cpp})
    source_group(TREE "${GAME_DIR}/include" PREFIX "Header Files" FILES ${${game}_hdr})

    set_property(TARGET ${game} PROPERTY CXX_STANDARD 20)

    target_include_directories(${game} PRIVATE
        "${GAME_DIR}/include"
    )

    target_link_libraries(${game} PRIVATE TanukiEngine)

    copy_fmod_dlls(${game})
    copy_engine_assets(${game})

    if(MSVC)
        target_compile_options(${game} PRIVATE /W4 /WX-)
    else()
        target_compile_options(${game} PRIVATE -Wall)
    endif()

    # SOURCE_ASSETS_DIR_R is game-specific — each game points to its own assets
    target_compile_definitions(${game} PRIVATE
        SOURCE_ASSETS_DIR_R="${GAME_DIR}/assets"
    )

    if(MSVC)
        set_target_properties(${game} PROPERTIES
            LINK_FLAGS_DEBUG   "/SUBSYSTEM:CONSOLE"
            LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
            VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${game}>"
        )
    endif()

    # -------------------------------------------------------------------
    # WaterBound-specific extras
    # -------------------------------------------------------------------
    if(game STREQUAL "WaterBound")
        # Copy game assets on top of engine assets
        add_custom_command(TARGET WaterBound POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${GAME_DIR}/assets" "$<TARGET_FILE_DIR:WaterBound>/assets"
            COMMENT "Copying WaterBound assets to output"
        )

        # Make shaders visible in VS IDE
        file(GLOB_RECURSE WB_SHADERS
            "${GAME_DIR}/assets/shaders/*.vert"
            "${GAME_DIR}/assets/shaders/*.frag"
            "${GAME_DIR}/assets/shaders/*.glsl"
        )
        if(WB_SHADERS)
            set_source_files_properties(${WB_SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
            target_sources(WaterBound PRIVATE ${WB_SHADERS})
        endif()
    endif()

endforeach()