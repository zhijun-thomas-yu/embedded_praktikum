#if !defined __SPI_C
#define __SPI_C

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "led.h"
#include "rfm69c.h"
#include "lcd.h"

void SPI_Initialize(void);
bool SPI_isEnabled(void);
void SPI_write(uint8_t address, uint8_t *data, uint8_t length);
void SPI_read(uint8_t address, uint8_t *dataBuffer, uint8_t length);
void SPI_test(void);

#endif