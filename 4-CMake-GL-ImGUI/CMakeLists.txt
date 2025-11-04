# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 2.8...3.13)

# Set the CMake policy version (this enables all policies up to CMake 3.5)
cmake_policy(VERSION 3.5)

project(WaterBound)

# Set the directory where custom CMake modules are stored
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Include project-wide utility and dependency modules
include(ImportDependencies)
importDependencies()

# Global compile flags and macros
add_definitions(-DGLEW_STATIC)
add_definitions(-DUSE_CSD3151_AUTOMATION=0) # Used for instructor's automation

# List of external libraries
set(ALL_LIBS
  PRIVATE glfw
  PRIVATE libglew_static
  PRIVATE imgui
  PRIVATE stb
  PRIVATE lua 
  PRIVATE rapidjson
)

# Include path for GLM (optional if not using FetchContent)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/glm)

# ======================= AUTO PROJECT DISCOVERY =========================

# Get all subdirectories in the projects folder
file(GLOB children RELATIVE "${CMAKE_CURRENT_LIST_DIR}/projects" "${CMAKE_CURRENT_LIST_DIR}/projects/*")




foreach(child ${children})
    if(IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/projects/${child}")
        message(STATUS "Configuring project: ${child}")

        # Collect sources
        file(GLOB_RECURSE ${child}_source_files
            ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/src/*.cpp
            ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/*.h
            ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/*.hpp
        )

        # Create executable
        add_executable(${child}
            ${${child}_source_files}
        )

        # Link libraries
        target_link_libraries(${child}
            ${ALL_LIBS}
        )

        # Set C++ standard
        set_property(TARGET ${child} PROPERTY CXX_STANDARD 20)

        # Warnings based on compiler
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${child} PRIVATE -Wall ${DisableWarnings})
        elseif (MSVC)
            target_compile_options(${child} PRIVATE /W4 /WX-)
        endif()

        # Include path for each project
        target_include_directories(${child} PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include
	    ${lua_SOURCE_DIR}
        )

        # ==== Shaders (copy to build dir so the exe can load them) ====
        set(${child}_SHADER_DIR "${CMAKE_CURRENT_LIST_DIR}/projects/${child}/shaders")

        # (Optional) If the shaders folder exists, copy it next to the target binary
        if (EXISTS "${${child}_SHADER_DIR}")
          # Show shader files in IDE (they're not compiled, just for convenience)
          file(GLOB_RECURSE ${child}_shader_files
               "${${child}_SHADER_DIR}/*.vert"
               "${${child}_SHADER_DIR}/*.frag"
               "${${child}_SHADER_DIR}/*.glsl")
          if(${child}_shader_files)
            set_source_files_properties(${${child}_shader_files} PROPERTIES HEADER_FILE_ONLY TRUE)
            target_sources(${child} PRIVATE ${${child}_shader_files})
          endif()

          # Copy the entire shaders directory into the runtime dir (next to the exe)
          add_custom_command(TARGET ${child} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${${child}_SHADER_DIR}"
                    "$<TARGET_FILE_DIR:${child}>/shaders"
            COMMENT "Copying shaders for ${child} to runtime directory"
          )

	  add_custom_command(TARGET ${child} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${${child}_SHADER_DIR}"
                    "${CMAKE_CURRENT_LIST_DIR}/build/shaders"
            COMMENT "Copying shaders for ${child} to runtime directory"
          )
          endif()
    endif()
endforeach()

target_include_directories(WaterBound PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound/assets)

target_include_directories(WaterBound PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound/lib/FMOD)

target_link_directories(WaterBound PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound/lib)

target_link_libraries(WaterBound PRIVATE
	fmod_vc.lib
	fmodL_vc.lib)

target_link_libraries(WaterBound PRIVATE lua)

add_custom_command(TARGET WaterBound POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound/lib/fmod.dll"
        "$<TARGET_FILE_DIR:WaterBound>/fmod.dll"

    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound/lib/fmodL.dll"
        "$<TARGET_FILE_DIR:WaterBound>/fmodL.dll"
)

# ==== Sync Scene folder for runtime ====
set(SCENE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound/shaders/Scene")

# Copy scenes to both build/shaders/Scene and build/Debug/shaders/Scene
add_custom_command(TARGET WaterBound POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:WaterBound>/shaders/Scene"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${SCENE_SOURCE_DIR}"
          "$<TARGET_FILE_DIR:WaterBound>/shaders/Scene"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${SCENE_SOURCE_DIR}"
          "${CMAKE_CURRENT_LIST_DIR}/build/Debug/shaders/Scene"
  COMMENT "Syncing Scene folder to both runtime and Debug directories"
)

# Absolute paths we want the game to know at compile time
set(WATERBOUND_RUNTIME_SCENE_DIR "$<TARGET_FILE_DIR:WaterBound>/shaders/Scene")
set(WATERBOUND_SOURCE_SCENE_DIR  "${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound/shaders/Scene")
set(WATERBOUND_SCENE_DIR  "${CMAKE_CURRENT_LIST_DIR}/projects/WaterBound/shaders")
set(WATERBOUND_RUNTIME_DIR  "$<TARGET_FILE_DIR:WaterBound>/shaders")

# Expose as preprocessor string literals usable in C++
target_compile_definitions(WaterBound PRIVATE
  RUNTIME_SCENE_DIR_R="$<JOIN:${WATERBOUND_RUNTIME_SCENE_DIR},>"
  SOURCE_SCENE_DIR_R="${WATERBOUND_SOURCE_SCENE_DIR}"
  SOURCE_DIR_R="${WATERBOUND_SCENE_DIR}"
  RUNTIME_DIR_R="$<JOIN:${WATERBOUND_RUNTIME_DIR},>"
)

# ======================= END AUTO CONFIGURATION =========================
