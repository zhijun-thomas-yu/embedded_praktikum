#include "led.h"

const uint8_t LED1 = 11;
const uint8_t LED2 = 14;
const uint8_t LED3 = 15;


uint8_t led_arr[] = {LED1, LED2, LED3};
uint8_t led_len = sizeof(led_arr) / sizeof(led_arr[0]);


void all_led_init(){
    for (uint8_t i=0; i<led_len; i++){
        gpio_init(led_arr[i]);
        // gpio set dir: true for Ausgang(output)
        gpio_set_dir(led_arr[i], true);
    }

}


void led_green(uint led_pin){
    gpio_init(led_pin);
    gpio_set_dir(led_pin, true);
    // The green light is on when pin is low level(false)
    gpio_put(led_pin, false);

}

void led_red(uint led_pin){
    gpio_init(led_pin);
    gpio_set_dir(led_pin, true);
    // The red light is on when pin is high level(true)
    gpio_put(led_pin, true);

}

void all_led_off(){
    for (uint8_t i=0; i<led_len; i++){
        // set pin as Eingang(input) to turn off this led pin
        gpio_set_dir(led_arr[i], false);
    }

}

void led_toggle(uint8_t led_pin){
    gpio_set_dir(led_pin, true);
    bool flag = gpio_get(led_pin); // get current led_pin level
    gpio_put(led_pin, !flag);
}

void led_debug_green(uint8_t error_code){
    /* if led_debugging_green(6), 
        6 is 110 in binary 
        then LED1(lowest bit) should be 0, LED1 green is not on 
        then LED2(middle bit) should be 1, LED2 green is on
        then LED3(highest bit) should be 1, LED3 green is on 
    */
    error_code = error_code & 0x07; // make sure the range of error code between 0 to 7
    all_led_off(); // wichtig
    for (uint8_t i=0; i<led_len; i++){
        if ((error_code >> i) & 0x01){
            led_green(led_arr[i]);
        }
    }
}
void led_debug_red(uint8_t error_code){
    error_code = error_code & 0x07;
    all_led_off();
    for (uint8_t i=0; i<led_len; i++){
        if ((error_code>>i) & 0x01){
            led_red(led_arr[i]);
        }
    }

}