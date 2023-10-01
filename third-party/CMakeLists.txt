######################################################################################################################
message(STATUS "Using SDL3 via add_subdirectory")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sdl ${CMAKE_CURRENT_SOURCE_DIR}/../out/sdl EXCLUDE_FROM_ALL)
######################################################################################################################



######################################################################################################################
message(STATUS "Using entt via add_subdirectory")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/entt ${CMAKE_CURRENT_SOURCE_DIR}/../out/entt EXCLUDE_FROM_ALL)
######################################################################################################################



######################################################################################################################
message(STATUS "Using glm via add_subdirectory")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm ${CMAKE_CURRENT_SOURCE_DIR}/../out/glm)
######################################################################################################################



######################################################################################################################
message(STATUS "Using UzlezzLanguage via add_subdirectory")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/uzlezz_language ${CMAKE_CURRENT_SOURCE_DIR}/../out/uzlezz_language)
######################################################################################################################



######################################################################################################################
message(STATUS "Using refl-cpp as single header file")
set_property(GLOBAL PROPERTY REFL_CPP_HEADERS_PROPERTY "${CMAKE_CURRENT_SOURCE_DIR}/refl-cpp/include")
######################################################################################################################