set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_library(Engine STATIC "source/engine.cpp" "include/uze/engine.h" "source/renderer/glad/gles3.h" "source/renderer/glad/gles3.cpp" "include/uze/renderer/shader.h" "include/uze/common.h" "source/renderer/shader.cpp" "include/uze/renderer/renderer.h" "source/renderer/renderer.cpp" "include/uze/renderer/buffer.h" "source/renderer/buffer.cpp" "include/uze/renderer/vertex_array.h" "source/renderer/vertex_array.cpp" "source/renderer/opengl.h")

set(ENGINE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(Engine PUBLIC ${ENGINE_HEADERS})
#target_include_directories(Engine PUBLIC "${CMAKE_SOURCE_DIR}/third-party/uzlezz_language/code/include")
target_include_directories(Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/sdl/include")
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/entt/")
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/glm")

target_link_libraries(Engine PRIVATE SDL3::SDL3)
target_link_libraries(Engine PUBLIC EnTT::EnTT)
#target_link_libraries(Engine PRIVATE UzlezzLanguage)

install(TARGETS Engine
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)