set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)



find_package(SDL3 QUIET)
if(SDL3_FOUND)
    message(STATUS "Using SDL3 via find_package")
endif()

# 2. Try using a vendored SDL library
if(NOT SDL3_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/ThirdParty/sdl/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/sdl out EXCLUDE_FROM_ALL)
    message(STATUS "Using SDL3 via add_subdirectory")
    set(SDL3_FOUND TRUE)
endif()

# 3. Download SDL, and use that.
if(NOT SDL3_FOUND)
    include(FetchContent)
    set(SDL_SHARED TRUE CACHE BOOL "Build a SDL shared library (if available)")
    set(SDL_STATIC TRUE CACHE BOOL "Build a SDL static library (if available)")
    FetchContent_Declare(
        SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main  # Replace this with a particular git tag or git hash
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    message(STATUS "Using SDL3 via FetchContent")
    FetchContent_MakeAvailable(SDL)
    set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-src" PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()


add_library(Engine "Source/Engine.cpp" "Include/UzE/Engine.h" "Source/glad/gles3.h" "Source/glad/gles3.cpp")

target_include_directories(Engine PUBLIC Include)

target_link_libraries(Engine PRIVATE SDL3::SDL3)

install(TARGETS Engine
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)