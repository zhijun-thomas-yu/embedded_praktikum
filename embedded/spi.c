#include "spi.h"

bool enabled = false;

void SPI_Initialize(void){
    // Initialisieren der SPI Kommunikation (Master Modus)
    spi_init(spi0, 500000);
    gpio_set_function(16, GPIO_FUNC_SPI); // MISO (Master In - Slavae Out) Data line to the master
    gpio_set_function(18, GPIO_FUNC_SPI); // SCLK: SPI Common clock line
    gpio_set_function(19, GPIO_FUNC_SPI); // MOSI (Master Out - Slave in) Data line to the master

    // NSS (low activate) : Slave select 
    gpio_init(17);
    gpio_set_dir(17, true);
    gpio_put(17, true);

    enabled = true; // SPI-Initialisierung erfolgreich
}



bool SPI_isEnabled(void){
    // The function returns not 0 as soon as the module is initialized
    return enabled;
}

void SPI_write(uint8_t address, uint8_t *data, uint8_t length){
    gpio_put(17, false); // Ziehen Sie NSS herunter, um die Kommunikation zu starten
    
    // Das höchstwertige Bit (MSB) wird auf 1 gesetzt, um anzuzeigen, dass es sich um einen Schreibvorgang handelt
    uint8_t writeAddr = address | 0x80; 

    // Schreibt Daten aus dem src Buffer der Länge len auf SPI Bus spi. Blockiert bis Übertragung vollständig abgewickelt ist.
    spi_write_blocking(spi0, &writeAddr, 1); // Über den SPI0-Port wird ein Byte gesendet, das in der Variablen writeAddr gespeichert ist.
    spi_write_blocking(spi0, data, length); // Zuerst wird die Registeradresse gesendet, danach werden die Daten fortlaufend übertragen

    gpio_put(17, 1); // NSS wird auf High gesetzt, um die Kommunikation zu beenden
}


void SPI_read(uint8_t address, uint8_t *dataBuffer, uint8_t length){
    gpio_put(17, false);

    uint8_t readAddr = address & 0x7F;

    // Zuerst wird die Registeradresse gesendet, danach werden die zurückgesendeten Daten gelesen
    spi_write_blocking(spi0, &readAddr, 1); 
    spi_read_blocking(spi0, 0, dataBuffer, length); 

    gpio_put(17, 1);

}

void SPI_test(void){
    // testSPI() is a test function defined in another file (e.g. rfm69c.c).
    // It writes data to the wireless module’s FIFO via SPI,
    // then reads it back to verify communication.
    // If the received data matches the sent data, the function returns true.
    //
    // → true  = communication OK  → turn on green LED
    // → false = communication error → turn on red LED
    //
    // while(1) keeps the program running after the test finishes.
    all_led_init();
    if(testSPI()){
            led_debug_green(7);
        }else led_debug_red(1);
    while (1);
}