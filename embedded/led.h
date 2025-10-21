#if !defined __LED_H
#define __LED_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

extern const uint8_t LED1;
extern const uint8_t LED2;
extern const uint8_t LED3;

void all_led_init();

void led_green(uint led_pin);
void led_red(uint led_pin);
void all_led_off();

void led_toggle(uint8_t led_pin);

void led_debug_green(uint8_t error_code);
void led_debug_red(uint8_t error_code);



#endif