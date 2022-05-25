cmake_minimum_required (VERSION 3.3)

project ("pic16f152-stubs"
	LANGUAGES CXX)

add_library (pic16f152-stubs STATIC
	"include/xc.h"
	"src/pic16f152.cpp")
target_include_directories (pic16f152-stubs
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
