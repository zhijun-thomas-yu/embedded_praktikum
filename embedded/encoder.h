#if !defined __Encoder_H
#define __Encoder_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <string.h>
#include "lcd.h"

void encoder_init(void);

void irq_handler_rotary(uint gpio, uint32_t events);

void rot_text_input_demo();


# endif