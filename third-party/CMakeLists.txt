######################################################################################################################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sdl ${CMAKE_CURRENT_SOURCE_DIR}/../out/sdl EXCLUDE_FROM_ALL)
message(STATUS "Using SDL3 via add_subdirectory")
######################################################################################################################



######################################################################################################################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/entt ${CMAKE_CURRENT_SOURCE_DIR}/../out/entt EXCLUDE_FROM_ALL)
message(STATUS "Using entt via add_subdirectory")
######################################################################################################################



######################################################################################################################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm ${CMAKE_CURRENT_SOURCE_DIR}/../out/glm)
message(STATUS "Using glm via add_subdirectory")
######################################################################################################################
