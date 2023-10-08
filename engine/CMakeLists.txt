set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_library(Engine SHARED "source/engine.cpp" "include/uze/engine.h" "source/renderer/glad/gles3.h" "source/renderer/glad/gl_impl.cpp" "include/uze/renderer/shader.h" "include/uze/common.h" "source/renderer/shader.cpp" "include/uze/renderer/renderer.h" "source/renderer/renderer.cpp" "include/uze/renderer/buffer.h" "source/renderer/buffer.cpp" "include/uze/renderer/vertex_array.h" "source/renderer/vertex_array.cpp" "source/renderer/opengl.h" "include/uze/log.h" "source/log.cpp" "source/renderer/glad/gl33.h" "include/uze/platform.h" "include/uze/core/scope.h" "include/uze/core/type_info.h" "source/core/type_info.cpp" "include/uze/core/job_system.h" "source/core/job_system.cpp" "include/uze/core/concurrent_queue.h" "include/uze/core/random.h" "source/core/random.cpp")

set(ENGINE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_definitions(-DUZE_EXPORT_DLL -DUZE_EXPORT)

target_include_directories(Engine PUBLIC ${ENGINE_HEADERS})
#target_include_directories(Engine PUBLIC "${CMAKE_SOURCE_DIR}/third-party/uzlezz_language/code/include")
target_include_directories(Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/sdl/include")
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/entt/")
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/glm")
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/uzlezz_language/third-party/fmt/include")

set_target_properties(Engine PROPERTIES OUTPUT_NAME "engine")

get_property(REFL_CPP_HEADERS GLOBAL PROPERTY REFL_CPP_HEADERS_PROPERTY)
target_include_directories(Engine PUBLIC ${REFL_CPP_HEADERS})

target_link_libraries(Engine PRIVATE SDL3::SDL3)
target_link_libraries(Engine PUBLIC EnTT::EnTT)
#target_link_libraries(Engine PRIVATE UzlezzLanguage)
target_link_libraries(Engine PUBLIC fmt)

install(TARGETS Engine
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin)