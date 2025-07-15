cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)


project(ReMapper C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.0")
message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# If you want debug output from USB (pass -DPICO_STDIO_USB=1) this ensures you don't lose any debug output while USB is set up
if (NOT DEFINED PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
set(PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 3000)
endif()

add_compile_options(-Wall
                    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
                    -Wno-unused-function # we have some for the docs that aren't called
                    )
                    
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
add_compile_options(-Wno-maybe-uninitialized)
endif()


# -----------------------------------------------------------------------------

# Initialize the SDK
pico_sdk_init()

# Create the executable from your main file
add_executable(ReMapper ReMapper.c)

# Link required libraries
target_link_libraries(ReMapper 
                    pico_stdlib
                    hardware_spi 
                    pico_unique_id 
                    tinyusb_host 
                    tinyusb_board)

target_include_directories(ReMapper PUBLIC "." "../../SHARED/include")

# Enable USB output, UART output, etc. (optional)
# pico_enable_stdio_usb(ReMapper 1)
# pico_enable_stdio_uart(ReMapper 0)

# Create a UF2 file for flashing
pico_add_extra_outputs(ReMapper)

