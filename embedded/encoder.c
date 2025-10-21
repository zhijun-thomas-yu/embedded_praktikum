#include "encoder.h"
const uint8_t ENC_A = 9;
const uint8_t ENC_B = 8;
const uint8_t ENC_SW = 10;

uint8_t enc_array[] = {ENC_A, ENC_B, ENC_SW};
// Phase A of the rotary encoder (ENC_A)
// Phase B of the rotary encoder (ENC_B)
// Button (ENC_SW)

// Record long press or short press events
volatile uint8_t event_button_long_press = 0;
volatile uint8_t event_button_short_press = 0;

// Record the direction of rotation (clockwise/counterclockwise)
volatile uint8_t event_rot_clkwise = 0;
volatile uint8_t event_rot_not_clkwise = 0;

// Store strings entered by the user (such as text input function)
char encoder_msg_str[33] = "";


void encoder_init(void){
    uint8_t enc_len = sizeof(enc_array) / sizeof(enc_array[0]);
    for (uint8_t i=0; i<enc_len; i++){
        gpio_init(enc_array[i]);
        // Set gpio in Input mode(Because the rotary encoder and buttons are external signals, Pico only needs to read)
        gpio_set_dir(enc_array[i], false);
        /* When the rotary encoder and button are grounded → the pin reads a low level (0); 
        when released → it becomes a high level (1) due to the pull-up */
        gpio_pull_up(enc_array[i]);
    }
    /* Pico SDK's interrupt callback is globally unique. 
    Once registered, the same callback will be invoked for interrupts from other GPIOs as well. */
    gpio_set_irq_enabled_with_callback(10, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true, &irq_handler_rotary);
    // ture: Enable interrupts

    // Phase A of the rotary encoder (ENC_A is GPIO 9)
    gpio_set_irq_enabled(9, GPIO_IRQ_EDGE_FALL, true);

}

void irq_handler_rotary(uint gpio, uint32_t events){
    // gpio: Which pin triggers the interrupt

    /* prev_interupt_time: Records the last interrupt time (static variable, holds the value)
    prev_pressed_time: Records the time the button was pressed (static variable) */
    static uint32_t prev_interupt_time = 0;
    static uint32_t prev_pressed_time = 0;

    // The time when the current interrupt occurs (unit: ms, accumulated since system startup)
    uint32_t curr_time = to_ms_since_boot(get_absolute_time());

    // 软件防抖 Entprellung
    if ((curr_time - prev_interupt_time)<30 && gpio != 20){
        // The interrupt of GPIO 20 (wireless module) is not subject to this restriction
        return;
    }else prev_interupt_time = curr_time;

    switch (gpio)
    {
    case ENC_A: // ENC_A Falling Edge
        if (gpio_get(ENC_B)){ // Read ENC_B
            event_rot_clkwise = 1;
            event_rot_not_clkwise = 0;
        }else{
            event_rot_clkwise = 0;
            event_rot_not_clkwise = 1;
        }
        break;
    
    case ENC_SW:
        switch (events)
        {
        case GPIO_IRQ_EDGE_FALL: // The moment you press the button
            prev_pressed_time = curr_time;
            break;
        case GPIO_IRQ_EDGE_RISE:
            if ((curr_time - prev_pressed_time)<1000){
                event_button_short_press = 1; // short press
            }else event_button_long_press = 1; // long press
            break;
        
        default:
            return;
        }
        break;

    default:
        break;
    }

}

void rot_text_input_demo(){
    const char *alphabet = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // Is a string constant of length(38), which will automatically end with a \0 (terminator) in C.
    uint8_t alph_len = (uint8_t) strlen(alphabet); // Just want to know the "number of valid characters" (excluding \0) here is 37

    char str[33];
    memset(str, ' ', sizeof(str)-1);
    uint8_t idx=0, pos=0;

    while (1){
        while ( !((event_button_long_press || event_button_short_press || event_rot_clkwise || event_rot_not_clkwise)))
        {
            sleep_ms(1);
        }

        if (event_button_long_press){event_button_long_press=0;}
        if (event_button_short_press){event_button_short_press=0; str[pos]=alphabet[idx]; pos++; idx=0;}

        if (event_rot_clkwise){event_rot_clkwise = 0; idx++;}
        if (event_rot_not_clkwise){event_rot_not_clkwise = 0; idx--;}

        // idx is uint8_t (0~255). When idx is 0 and you execute idx--, it will not become -1, but overflow to 255 (0xFF)
        if (idx==0xFF){
            idx = alph_len-1;
        }else idx %= alph_len;

        /* pos indicates the position of the input string (the column in which the cursor is located).
        char str[33] is a 33-byte array, which can hold 32 characters (the last digit is reserved for the terminating character, \0 ).
        Therefore, the legal range is 0 to 31. */
        if (pos>31){
            pos = 0;
        }else if(pos>15 && pos <=31){
            lcd_locate(1, pos-16); 
        }else lcd_locate(0, pos);

        lcd_writeData(alphabet[idx]);

    }

}