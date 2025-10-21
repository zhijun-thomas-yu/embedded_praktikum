#include "lcd.h"

const uint8_t CONTER_PIN = 0;
const uint8_t E_PIN = 1;
const uint8_t RW_PIN = 2;
const uint8_t RS_PIN = 3;
const uint8_t D4_PIN = 4;
const uint8_t D5_PIN = 5;
const uint8_t D6_PIN = 6;
const uint8_t D7_PIN = 7;



void lcd_writeNibble(char wNibble){
    // Schreibt ein Nibble an das LCD(Datenübertragung vom Mikrocontroller zum LCD-Modul)
    // nibble = 4 bits such as wNibble = 0b1011
    /* char is an 8-bit integer type, usually used to represent characters.
    It can also be used as an integer in the range of 0 to 255 (or -128 to 127), so it can perform bit operations, shift operations, etc. */
    const uint8_t DataPins_arr[] = {D4_PIN, D5_PIN, D6_PIN, D7_PIN};
    uint8_t len_Nibble = 4;
    /* D7 ← bit3
    D6 ← bit2
    D5 ← bit1
    D4 ← bit0 */
    /* if wNibble=1011, 
    i=3, wNibble >> 3: 0b0001 (because shift right 3 bits, the lowest three bits 010 of wNibble are shifted out) 
    0b0001 & 0b0001 = 1 (store in pin D7)
    i=2, wNibble >> 2: 0b0010 (because shift right 2 bits, the lowest two bits 10 of wNibble are shifted out) 
    0b0010 & 0b0001 = 0 (store in pin D6)
    i=1, wNibble >> 1: 0b0101 (because shift right 1 bits, the lowest bit 0 of wNibble are shifted out) 
    0b0101 & 0b0001 = 1 (store in pin D5)
    i=0, wNibble >> 1: 0b1011 (becuase shift right 0 bit, which means no shift)
    0b1011 & 0b0001 = 1 (store in pin D4)    */
    for (uint8_t i=0; i<len_Nibble; i++){
        gpio_put(DataPins_arr[i],  (wNibble>>i)&0b01);
    }
    sleep_ms(5);

    /* Fallenden Flanke: E from high level to low level
    high level should be keept more than 230ns; 
    than convert it to low level, and low level should be keept more than 10ns */
    gpio_put(E_PIN, 1);
    sleep_ms(5);
    gpio_put(E_PIN, 0);
    sleep_ms(5);
}


void lcd_writeByte(char wByte){
    uint8_t MSB = (wByte>>4) & 0x0F; // Make sure the upper 4 bits are shifted to the lower 4 bits and mask the other bits
    lcd_writeNibble(MSB);
    uint8_t LSB = wByte & 0x0F; // Directly take the lower 4 bits
    lcd_writeNibble(LSB);
}


void lcd_writeCommand(char wCommand){
    // Schreibt ein Byte in das Command=Register des LCD
    // if Select Register is low level, then it is Command register, we need to write command
    gpio_put(RS_PIN, 0);
    lcd_writeByte(wCommand);
    sleep_ms(5);
}


void lcd_writeData(char wData){
    // Schreibt ein Byte in das Data=Register des LCD
    // if Select Register is high level, then it is Data register, we need to write data
    gpio_put(RS_PIN, 1);
    lcd_writeByte(wData);
    sleep_ms(5);
}

void lcd_init(void){
    sleep_ms(50); //  Nachdem die Stromversorgung angelegt wurde ca. 50ms warten bis sich das Modul intern vorinitialisiert hat

    // Initialization of all LCD PIN
    const uint8_t lcd_arr[] = {CONTER_PIN, E_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN};
    const uint8_t  lcd_arr_len = sizeof(lcd_arr) / sizeof(lcd_arr[0]);
    for (uint8_t i=0; i<lcd_arr_len; i++){
        gpio_init(lcd_arr[i]);
        gpio_set_dir(lcd_arr[i], true);
    }

    /* To be on the saft side, switch the module back to 8-bit mode, 
    according to the Function Set from Command list of HD44780, for 8 mode, DL(data length) of Function set in DB4 should be 1, 
    lcd_writeCommand(char wCommand): Sends a complete Byte (but only used after entering 4-bit mode), can only send nibbles before initialization
    Now only send the upper 4 digits(MSB), which is DB7(0) DB6(0) DB5(1) DB4(DL), DL is 1 for 8 bit mode
    so it is 0x3(we can also say it is 0x03)*/
    lcd_writeCommand(0x3);
    sleep_ms(5);
    lcd_writeCommand(0x3);
    sleep_ms(5);
    lcd_writeCommand(0x3);
    sleep_ms(5);

    // switch the module to 4-bit mode
    /* lcd_writeCommand(char wCommand): Sends a complete Byte (but only used after entering 4-bit mode) can only send nibbles before initialization
    Now only send the upper 4 digits(MSB), which is DB7(0) DB6(0) DB5(1) DB4(DL), DL is 0 for 4 bit mode
    so it is 0x2(we can also say it is 0x02)*/
    lcd_writeCommand(0x2);
    sleep_ms(5);

    // so bits in binary of Fucntion Set( DB7(0) DB6(0) DB5(1) DB4(DL) DB3(N) DB2(F) DB1(-) DB0(-) 
    // should be DB7(0) DB6(0) DB5(1) DB4(0) DB3(1) DB2(0) DB1(0) DB0(0) to select connected display)
    // i.e. 0x28 
    lcd_writeCommand(0x28);
    sleep_ms(5);

    // activate display, switch on cursor, switch on flashing(see Display on/off control from Command list of HD44780)
    lcd_writeCommand(0x0F);
    sleep_ms(5);

    lcd_writeCommand(0x01); // clear display

}

void lcd_print(char *str){
    // char *str is the first address of the string, *str is the current character obtained by dereferencing the string
    while(*str){
        // continue as long as the current character is not \0 (the end of the string)
        // \0 is called the null character, it represents a character with an ASCII value of 0
        lcd_writeData(*str);
        str++;
    }
}


void lcd_locate(uint8_t row, uint8_t col){
    /* According to Figure 6.6, the command of Set DDRAM address is based on D0-D7,
    D7 should always be 1, D0-D6 represent the address(Add) of the cursor position
    for exmaple: Address of line2, column3 should be 0x42, whichn means D6(1) D5(0) D4(0) D3(0) D2(1) D1(0)
    so D7(1 )D6(1) D5(0) D4(0) D3(0) D2(1) D1(0) is 0xC2*/

    uint8_t DDRAM_address = col & 0x0F; // Since the maximal column in our display is 16, we set a Limit columns to 0~15
    if (row==1){
        DDRAM_address = 0x40 + col;
    }
    else DDRAM_address = DDRAM_address; // when row=0, DDRAM_address is the value of column

    // we need to send a Byte(D7~D0 in total 8-bits) in command, so use the 7-bits DDRAM_address(D6~D0) OR 0x80, because D7 is always 1
    lcd_writeCommand(DDRAM_address | 0x80);
}

void lcd_init_pwm(uint16_t wrap, uint16_t chan_lvl){
    // Set GPIO 0 to PWM function
    gpio_set_function(0, GPIO_FUNC_PWM);

    /* The Pico RP2040 chip has a total of 8 PWM slices.
    Each slice can independently output two PWM signals, called channels A and B. So you can have up to 16 PWM outputs. */

    // Get the slice and channel corresponding to GPIO 0
    uint slice = pwm_gpio_to_slice_num(0);
    uint channel = pwm_gpio_to_channel(0);

    // Set the maximum value that the counter can reach(wrap value)
    pwm_set_wrap(slice, wrap);

    // Set the threshold that determines the duty cycle
    pwm_set_chan_level(slice, channel, chan_lvl);

    // The frequency of the PWM signal should be 4 KHz 
    // Clock frequency of the PICO system is 125MHz
    // PWM Cycle duration = (wrap+1) × System cycle duration
    // PWM frequencty = System clock frequency/(wrap + 1), so wrap = 125MHz/4kHz -1 = 31249

    // level(threshold) = wrap × duty_cycle(31249 x 1.5% =469)

    pwm_set_enabled(slice, true);

}

    void lcd_cycle_pwm(){
        const uint16_t wrap = 31249;
        const uint slice = pwm_gpio_to_slice_num(0);
        const uint channel = pwm_gpio_to_channel(0);

        lcd_locate(0, 0);
        lcd_print("Hallo Tamia:)");
        lcd_locate(1, 0);
        lcd_print("Have a nice day!");

        for (uint8_t dc=0; dc<101; dc++){
            // level(threshold) = wrap × duty_cycle(31249 x 1.5% =469)
            uint16_t chan_level = wrap * dc / 100;
            pwm_set_chan_level(slice, channel, chan_level);
            printf("Duty Cycle: %d%%, Level:%d\n", dc, chan_level);
            sleep_ms(500);
        }

    }







