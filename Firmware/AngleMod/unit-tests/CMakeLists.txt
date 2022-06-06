cmake_minimum_required (VERSION 3.15)

project ("unit-tests"
    LANGUAGES C CXX
    VERSION "0.0.1")

add_subdirectory ("../pic16f152-stubs" "pic16f152-stubs")
add_subdirectory ("thirdparty/googletest")

set (INSTALL_GTEST OFF CACHE INTERNAL "")

set (PIC16_SOURCES
	"../AngleMod.X/src/adc.c"
	"../AngleMod.X/src/btn.c"
	"../AngleMod.X/src/cli.c"
	"../AngleMod.X/src/config.c"
	"../AngleMod.X/src/dac.c"
	"../AngleMod.X/src/gpio.c"
	"../AngleMod.X/src/joy.c"
	"../AngleMod.X/src/math.c"
	"../AngleMod.X/src/osc.c"
	"../AngleMod.X/src/rb.c"
	"../AngleMod.X/src/seq.c"
	"../AngleMod.X/src/uart.c"
	"../AngleMod.X/src/main.c")
set (PIC16_HEADERS
	"../AngleMod.X/include/anglemod/adc.h"
	"../AngleMod.X/include/anglemod/btn.h"
	"../AngleMod.X/include/anglemod/cli.h"
	"../AngleMod.X/include/anglemod/config.h"
	"../AngleMod.X/include/anglemod/dac.h"
	"../AngleMod.X/include/anglemod/gpio.h"
	"../AngleMod.X/include/anglemod/joy.h"
	"../AngleMod.X/include/anglemod/math.h"
	"../AngleMod.X/include/anglemod/osc.h"
	"../AngleMod.X/include/anglemod/rb.h"
	"../AngleMod.X/include/anglemod/seq.h"
	"../AngleMod.X/include/anglemod/uart.h")
set_source_files_properties (${PIC16_SOURCES} PROPERTIES 
	LANGUAGE CXX)
add_executable (unit-tests
	${PIC16_HEADERS}
	${PIC16_SOURCES})
target_include_directories (unit-tests
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../AngleMod.X/include>)
target_compile_definitions (unit-tests
	PRIVATE GTEST_TESTING)
target_link_libraries (unit-tests PRIVATE pic16f152-stubs)
target_link_libraries (unit-tests PRIVATE gmock gmock_main)
