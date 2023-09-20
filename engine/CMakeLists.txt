set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)



find_package(SDL3 QUIET)
if(SDL3_FOUND)
    message(STATUS "Using SDL3 via find_package")
endif()

# 2. Try using a vendored SDL library
if(NOT SDL3_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/third-party/sdl/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/third-party/sdl ${CMAKE_SOURCE_DIR}/third-party/sdl/out EXCLUDE_FROM_ALL)
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
    FetchContent_MakeAvailable(sdl)
    set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-src" PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()


find_package(SDL3MIXER QUIET)
if(SDL3MIXER_FOUND)
    message(STATUS "Using SDL3_mixer via find_package")
endif()

# 2. Try using a vendored SDL library
if(NOT SDL3MIXER_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/third-party/sdl_mixer/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/third-party/sdl_mixer ${CMAKE_SOURCE_DIR}/third-party/sdl_mixer/out EXCLUDE_FROM_ALL)
    message(STATUS "Using SDL3_mixer via add_subdirectory")
    set(SDL3MIXER_FOUND TRUE)
endif()

# 3. Download SDL_mixer, and use that.
if(NOT SDL3MIXER_FOUND)
    include(FetchContent)
    set(SDL_SHARED TRUE CACHE BOOL "Build a SDL_mixer shared library (if available)")
    set(SDL_STATIC TRUE CACHE BOOL "Build a SDL_mixer static library (if available)")
    FetchContent_Declare(
        SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG main  # Replace this with a particular git tag or git hash
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    message(STATUS "Using SDL3_mixer via FetchContent")
    FetchContent_MakeAvailable(sdl_mixer)
    set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-mixer-src" PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()


#add_subdirectory(${CMAKE_SOURCE_DIR}/third-party/uzlezz_language ${CMAKE_SOURCE_DIR}/third-party/uzlezz_language/out)
#message(STATUS "Using UzlezzLanguage via add_subdirectory")

find_package(entt QUIET)
if(entt_FOUND)
    message(STATUS "Using entt via find_package")
endif()

if(NOT entt_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/third-party/entt/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/third-party/entt ${CMAKE_SOURCE_DIR}/third-party/entt/out EXCLUDE_FROM_ALL)
    message(STATUS "Using entt via add_subdirectory")
    set(entt_FOUND TRUE)
endif()

if(NOT entt_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        SDL
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    message(STATUS "Using entt via FetchContent")
    FetchContent_MakeAvailable(entt)
    set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/entt-src" PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()

add_library(Engine STATIC "source/engine.cpp" "include/uze/engine.h" "source/renderer/glad/gles3.h" "source/renderer/glad/gles3.cpp" "include/uze/renderer/shader.h" "include/uze/common.h" "source/renderer/shader.cpp" "include/uze/renderer/renderer.h" "source/renderer/renderer.cpp" "include/uze/renderer/buffer.h" "source/renderer/buffer.cpp")

target_include_directories(Engine PUBLIC include)
#target_include_directories(Engine PUBLIC "${CMAKE_SOURCE_DIR}/third-party/uzlezz_language/code/include")

target_link_libraries(Engine PRIVATE SDL3::SDL3)
target_link_libraries(Engine PRIVATE SDL3_mixer::SDL3_mixer)
target_link_libraries(Engine PUBLIC EnTT::EnTT)
#target_link_libraries(Engine PRIVATE UzlezzLanguage)

install(TARGETS Engine
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)