set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Editor "source/entry_point.cpp")

set(EDITOR_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/source)

target_include_directories(Editor PRIVATE ${EDITOR_HEADERS})
target_include_directories(Editor PRIVATE ${ENGINE_HEADERS})
target_link_libraries(Editor Engine)

install(TARGETS Editor RUNTIME DESTINATION bin)