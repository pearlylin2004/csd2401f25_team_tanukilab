include(FetchContent)

# ===================== Import GLFW =====================
macro(import_glfw)
    if(NOT TARGET glfw)
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        if(NOT glfw_POPULATED)
            set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            FetchContent_Populate(glfw)
        endif()

        add_subdirectory(${glfw_SOURCE_DIR})
        include_directories(${GLFW_SOURCE_DIR}/include)
    endif()
endmacro()

# ===================== Import GLM =====================
macro(import_glm)
    if(NOT TARGET glm)
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(glm)
        include_directories(${glm_SOURCE_DIR})
    endif()
endmacro()

# ===================== Import GLEW =====================
macro(import_glew)
    if(NOT TARGET glew_s)
        FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
            GIT_TAG d06782b910213d675925e6e51a69ad0fd1fe1f23
        )
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
        FetchContent_MakeAvailable(glew)
        include_directories(${glew_SOURCE_DIR}/include)
    endif()
endmacro()

# ===================== Import ImGui =====================
macro(import_imgui)
    if(NOT TARGET imgui)
        FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG docking
        )
        if(NOT imgui_POPULATED)
            FetchContent_Populate(imgui)
        endif()

        set(IMGUI_SOURCES
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )

        add_library(imgui STATIC ${IMGUI_SOURCES})
        target_include_directories(imgui PUBLIC 
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
        )
        target_link_libraries(imgui PUBLIC glfw libglew_static)
        set_property(TARGET imgui PROPERTY CXX_STANDARD 11)
        set_property(TARGET imgui PROPERTY CXX_STANDARD_REQUIRED ON)
    endif()
endmacro()

# ===================== Import stb (header-only) =====================
macro(import_stb)
    if(NOT TARGET stb)
        FetchContent_Declare(
            stb
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(stb)
        add_library(stb INTERFACE)
        target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
    endif()
endmacro()

# ===================== Import Lua =====================
macro(import_lua)
    if(NOT TARGET lua)
        FetchContent_Declare(
            lua
            GIT_REPOSITORY https://github.com/lua/lua.git
            GIT_TAG v5.4.7
        )
        FetchContent_Populate(lua)

        set(LUA_SOURCES
            ${lua_SOURCE_DIR}/lapi.c
            ${lua_SOURCE_DIR}/lcode.c
            ${lua_SOURCE_DIR}/lctype.c
            ${lua_SOURCE_DIR}/ldebug.c
            ${lua_SOURCE_DIR}/ldo.c
            ${lua_SOURCE_DIR}/ldump.c
            ${lua_SOURCE_DIR}/lfunc.c
            ${lua_SOURCE_DIR}/lgc.c
            ${lua_SOURCE_DIR}/llex.c
            ${lua_SOURCE_DIR}/lmem.c
            ${lua_SOURCE_DIR}/lobject.c
            ${lua_SOURCE_DIR}/lopcodes.c
            ${lua_SOURCE_DIR}/lparser.c
            ${lua_SOURCE_DIR}/lstate.c
            ${lua_SOURCE_DIR}/lstring.c
            ${lua_SOURCE_DIR}/ltable.c
            ${lua_SOURCE_DIR}/ltm.c
            ${lua_SOURCE_DIR}/lundump.c
            ${lua_SOURCE_DIR}/lvm.c
            ${lua_SOURCE_DIR}/lzio.c
            ${lua_SOURCE_DIR}/lauxlib.c
            ${lua_SOURCE_DIR}/lbaselib.c
            ${lua_SOURCE_DIR}/lcorolib.c
            ${lua_SOURCE_DIR}/ldblib.c
            ${lua_SOURCE_DIR}/liolib.c
            ${lua_SOURCE_DIR}/lmathlib.c
            ${lua_SOURCE_DIR}/loslib.c
            ${lua_SOURCE_DIR}/lstrlib.c
            ${lua_SOURCE_DIR}/ltablib.c
            ${lua_SOURCE_DIR}/lutf8lib.c
            ${lua_SOURCE_DIR}/loadlib.c
            ${lua_SOURCE_DIR}/linit.c
        )

        add_library(lua STATIC ${LUA_SOURCES})
        target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})
        set_property(TARGET lua PROPERTY C_STANDARD 99)
    endif()
endmacro()

# ===================== Import rapidjson =====================
macro(import_rapidjson)
    if(NOT TARGET rapidjson)
        include(FetchContent)

        # 1. Declare RapidJSON
        FetchContent_Declare(
            rapidjson_src
            GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
            GIT_TAG master          # or a known tag like v1.1.0 if you want stability
        )

        # 2. Download it
        FetchContent_GetProperties(rapidjson_src)
        if(NOT rapidjson_src_POPULATED)
            FetchContent_Populate(rapidjson_src)
        endif()

        # 3. Make a header-only interface target
        add_library(rapidjson INTERFACE)

        # 4. Point that target at the include/ directory inside RapidJSON
        target_include_directories(rapidjson INTERFACE
            ${rapidjson_src_SOURCE_DIR}/include
        )
    endif()
endmacro()

# ===================== Import All Dependencies =====================
macro(importDependencies)
    message(STATUS "Starting to import dependencies...")

    message(STATUS "Importing GLFW...")
    import_glfw()
    message(STATUS "GLFW imported successfully.")

    message(STATUS "Importing GLM...")
    import_glm()
    message(STATUS "GLM imported successfully.")

    message(STATUS "Importing GLEW...")
    import_glew()
    message(STATUS "GLEW imported successfully.")

    message(STATUS "Importing ImGui...")
    import_imgui()
    message(STATUS "ImGui imported successfully.")

    message(STATUS "Importing stb...")
    import_stb()
    message(STATUS "stb imported successfully.")

    message(STATUS "Importing Lua...")
    import_lua()
    message(STATUS "Lua imported successfully.")

     message(STATUS "Importing RapidJson...")
    import_rapidjson()
    message(STATUS "RapidJson imported successfully.")

    message(STATUS "All dependencies have been imported successfully.")
endmacro()
