#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "led.h"
#include "lcd.h"
#include "encoder.h"
#include "spi.h"

// int main(){
//     stdio_usb_init();
//     stdio_init_all();

//     all_led_init();
//     all_led_off();

//     lcd_init();
//     lcd_init_pwm(31249, 0);

//     encoder_init();

//     while (1)
//     {
//         rot_text_input_demo();
//     }

// }

int main(){
    stdio_usb_init();
    stdio_init_all();

    all_led_init();
    all_led_off();

    lcd_init();
    lcd_init_pwm(31249, 0);

    encoder_init();

    while (1)
    {
        SPI_test();
    }

}