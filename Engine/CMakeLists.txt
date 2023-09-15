add_library(Engine "Source/Engine.cpp" "Include/UzE/Engine.h")

target_include_directories(Engine PUBLIC Include)

install(TARGETS Engine
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)