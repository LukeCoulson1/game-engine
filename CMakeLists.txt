cmake_minimum_required(VERSION 3.16)
project(GameEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Print build configuration info
message(STATUS "Building Game Engine...")
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

# Find packages using vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

find_package(glm CONFIG REQUIRED)
if(NOT glm_FOUND)
    message(FATAL_ERROR "GLM not found! Please install GLM mathematics library or use vcpkg.")
endif()

find_package(nlohmann_json CONFIG REQUIRED)
if(NOT nlohmann_json_FOUND)
    message(FATAL_ERROR "nlohmann_json not found! Please install nlohmann/json library or use vcpkg.")
endif()

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(src)

# Check for ImGui
set(IMGUI_DIR "external/imgui")
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${IMGUI_DIR}/imgui.h")
    message(FATAL_ERROR "Dear ImGui not found in ${IMGUI_DIR}/! Please download ImGui and extract to external/imgui/")
endif()

include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

# ImGui sources
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

# Engine source files
file(GLOB_RECURSE ENGINE_SOURCES 
    "src/core/*.cpp"
    "src/graphics/*.cpp"
    "src/audio/*.cpp"
    "src/input/*.cpp"
    "src/scene/*.cpp"
    "src/components/*.cpp"
    "src/systems/*.cpp"
    "src/utils/*.cpp"
    "src/generation/*.cpp"
    "src/rendering/*.cpp"
)

# Editor source files
file(GLOB_RECURSE EDITOR_SOURCES 
    "src/editor/*.cpp"
)

# Create engine library (including editor sources for GameEditor build)
add_library(GameEngineLib STATIC ${ENGINE_SOURCES} ${IMGUI_SOURCES} ${EDITOR_SOURCES})

# Link libraries using vcpkg targets
target_link_libraries(GameEngineLib 
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2main>,SDL2::SDL2main,SDL2::SDL2main-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
    glm::glm
    nlohmann_json::nlohmann_json
)

# Create editor executable
add_executable(GameEditor src/main.cpp)
target_link_libraries(GameEditor GameEngineLib)

# Create runtime executable
add_executable(GameRuntime src/runtime.cpp)
target_link_libraries(GameRuntime GameEngineLib)

# Set output directories
set_target_properties(GameEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(GameRuntime PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy assets to build directory if they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif()

# Platform-specific settings
if(WIN32)
    # Copy SDL2 DLLs on Windows (if using vcpkg)
    if(DEFINED ENV{VCPKG_ROOT})
        message(STATUS "Using vcpkg - DLLs will be copied automatically")
    endif()
    
    # Set subsystem for Windows (removes console window for release)
    # set_target_properties(GameEditor PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Print success message
message(STATUS "Game Engine configuration completed successfully!")
message(STATUS "Build targets: GameEditor, GameRuntime")
