cmake_minimum_required (VERSION 3.3)

project ("cli-sim"
    LANGUAGES C CXX
    VERSION "0.0.1")

add_subdirectory ("../pic16f152-stubs" "pic16f152-stubs")

set (PIC16_SOURCES
	"../AngleMod.X/src/adc.c"
	"../AngleMod.X/src/btn.c"
	"../AngleMod.X/src/cli.c"
	"../AngleMod.X/src/config.c"
	"../AngleMod.X/src/dac.c"
	"../AngleMod.X/src/gpio.c"
	"../AngleMod.X/src/joy.c"
	"../AngleMod.X/src/math.c"
	"../AngleMod.X/src/seq.c"
	"../AngleMod.X/src/uart.c"
	"../AngleMod.X/src/main.c")
set (PIC16_HEADERS
	"../AngleMod.X/include/anglemod/adc.h"
	"../AngleMod.X/include/anglemod/btn.h"
	"../AngleMod.X/include/anglemod/cli.h"
	"../AngleMod.X/include/anglemod/config.h"
	"../AngleMod.X/include/anglemod/seq.h"
	"../AngleMod.X/include/anglemod/dac.h"
	"../AngleMod.X/include/anglemod/gpio.h"
	"../AngleMod.X/include/anglemod/joy.h"
	"../AngleMod.X/include/anglemod/log.h"
	"../AngleMod.X/include/anglemod/math.h"
	"../AngleMod.X/include/anglemod/rb.h"
	"../AngleMod.X/include/anglemod/uart.h")
set_source_files_properties (${PIC16_SOURCES} PROPERTIES 
	LANGUAGE CXX)
add_executable (cli-sim
	${PIC16_HEADERS}
	${PIC16_SOURCES}
	"src/main.cpp")
target_include_directories (cli-sim
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../AngleMod.X/include>)
target_compile_definitions (cli-sim
	PRIVATE CLI_SIM)
target_link_libraries (cli-sim 
	PRIVATE
		pic16f152-stubs
		$<$<PLATFORM_ID:Windows>:Winmm>)
