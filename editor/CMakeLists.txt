set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Editor "source/entry_point.cpp")

set(EDITOR_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/source)

target_include_directories(Editor PRIVATE ${EDITOR_HEADERS})
target_include_directories(Editor PRIVATE ${ENGINE_HEADERS})
target_include_directories(Editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/uzlezz_language/third-party/fmt/include")

target_link_libraries(Editor Engine)
target_link_libraries(Editor fmt)

if(EMSCRIPTEN)
	target_link_options(Editor PRIVATE -sEXPORTED_FUNCTIONS=['_main']
		-sUSE_WEBGL2=1 -sMIN_WEBGL_VERSION=2 -sFULL_ES3=1)
endif()

install(TARGETS Editor RUNTIME DESTINATION bin)