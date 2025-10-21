#if !defined __LCD_H
#define __LCD_H


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"


void lcd_writeNibble(char wNibble);
void lcd_writeByte(char wByte);
void lcd_writeCommand(char wCommand);
void lcd_writeData(char wData);

void lcd_init(void);

void lcd_print(char *str);
void lcd_locate(uint8_t row, uint8_t col);

void lcd_init_pwm(uint16_t wrap, uint16_t chan_lvl);
void lcd_cycle_pwm();

#endif