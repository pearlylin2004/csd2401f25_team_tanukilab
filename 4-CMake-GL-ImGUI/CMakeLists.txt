# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.20)          # Updated to avoid policy warnings
project(WaterBound)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ImportDependencies)
importDependencies()

add_definitions(-DGLEW_STATIC)
add_definitions(-DUSE_CSD3151_AUTOMATION=0)

set(ALL_LIBS
  PRIVATE glfw
  PRIVATE libglew_static
  PRIVATE imgui
  PRIVATE stb
  PRIVATE lua
  PRIVATE rapidjson
  PRIVATE freetype
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/glm)

# ======================= AUTO PROJECT DISCOVERY =========================
file(GLOB children RELATIVE "${CMAKE_CURRENT_LIST_DIR}/projects"
     "${CMAKE_CURRENT_LIST_DIR}/projects/*")

foreach(child ${children})
  if(IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/projects/${child}")
    message(STATUS "Configuring project: ${child}")

    file(GLOB_RECURSE ${child}_source_files
         ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/src/*.cpp
         ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/*.h
         ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/*.hpp)

# ===================== Filter for editor files in VS =====================
    file(GLOB_RECURSE ${child}_editor_headers
        ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/Editor/*.h
        ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/Editor/*.hpp
    )

    file(GLOB_RECURSE ${child}_editor_sources
        ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/src/Editor/*.cpp
    )

    source_group("Header Files\\Editor" FILES ${${child}_editor_headers})
    source_group("Source Files\\Editor" FILES ${${child}_editor_sources})

    list(APPEND ${child}_source_files
        ${${child}_editor_headers}
        ${${child}_editor_sources}
    )
# ========================================================================

    add_executable(${child} ${${child}_source_files})
    target_link_libraries(${child} ${ALL_LIBS})
    set_property(TARGET ${child} PROPERTY CXX_STANDARD 20)

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      target_compile_options(${child} PRIVATE -Wall ${DisableWarnings})
    elseif (MSVC)
      target_compile_options(${child} PRIVATE /W4 /WX-)
    endif()

    target_include_directories(${child} PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include
      ${lua_SOURCE_DIR}
    )
  endif()
endforeach()

# ======================= WaterBound-specific config ====================
set(WB_DIR        "${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound")
set(WB_ASSETS_DIR "${WB_DIR}/assets")

# Include + link (FMOD)
target_include_directories(WaterBound PRIVATE
  "${WB_DIR}/assets"
  "${WB_DIR}/lib/FMOD"
)
target_link_directories(WaterBound PRIVATE "${WB_DIR}/lib")
target_link_libraries(WaterBound PRIVATE fmod_vc.lib fmodL_vc.lib lua)

# FMOD DLLs next to exe
add_custom_command(TARGET WaterBound POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
          "${WB_DIR}/lib/fmod.dll"  "$<TARGET_FILE_DIR:WaterBound>/fmod.dll"
  COMMAND ${CMAKE_COMMAND} -E copy
          "${WB_DIR}/lib/fmodL.dll" "$<TARGET_FILE_DIR:WaterBound>/fmodL.dll"
)

# Make shaders visible in IDE (not compiled)
file(GLOB_RECURSE WB_SHADER_FILES
     "${WB_ASSETS_DIR}/shaders/*.vert"
     "${WB_ASSETS_DIR}/shaders/*.frag"
     "${WB_ASSETS_DIR}/shaders/*.glsl")
if(WB_SHADER_FILES)
  set_source_files_properties(${WB_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
  target_sources(WaterBound PRIVATE ${WB_SHADER_FILES})
endif()

# FIXED: No more cyclic custom target!
# Assets copied using POST_BUILD instead of a separate target
add_custom_command(TARGET WaterBound POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${WB_ASSETS_DIR}" "$<TARGET_FILE_DIR:WaterBound>/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${WB_ASSETS_DIR}" "${CMAKE_BINARY_DIR}/assets"
  COMMENT "Copying WaterBound assets to runtime and build directories"
)

# Compile-time path macros used by your C++ code
target_compile_definitions(WaterBound PRIVATE
  RUNTIME_ASSETS_DIR_R="./assets"
  RUNTIME_SHADER_DIR_R="./assets/shaders"
  RUNTIME_SCENE_DIR_R="./assets/Scene"
  RUNTIME_PREFAB_DIR_R="./assets/Prefab"
  RUNTIME_AUDIO_DIR_R="./assets/Audio"
  RUNTIME_SCRIPT_DIR_R="./assets/scripting"
  RUNTIME_RES_DIR_R="./assets/resources"
  SOURCE_ASSETS_DIR_R="${WB_ASSETS_DIR}"
)

# ======================= END AUTO CONFIGURATION =========================
