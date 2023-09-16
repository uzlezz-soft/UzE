set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Editor "source/entry_point.cpp")

target_include_directories(Editor PRIVATE ${CMAKE_SOURCE_DIR}/engine)
target_link_libraries(Editor Engine)

install(TARGETS Editor RUNTIME DESTINATION bin)