cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(embedded C CXX ASM)
pico_sdk_init()
add_executable(embedded
	embedded.c
	led.c
	lcd.c
	encoder.c
	rfm69c.c
	spi.c

)
pico_enable_stdio_usb(embedded 1)
pico_enable_stdio_uart(embedded 1)
pico_add_extra_outputs(embedded)
target_link_libraries(embedded pico_stdlib hardware_pwm hardware_spi)
 
