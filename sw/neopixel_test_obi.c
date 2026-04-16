#include <stdint.h>
#include "uart.h"
#include "print.h"
#include "util.h"
#include "neopixel.h"
#include "timer.h"

int main(void) {
    uart_init();
    printf("NeoPixel OBI test\n");
    uart_write_flush();

    neopixel_init();

    // write colors directly via OBI FIFO
    neopixel_fifo_write(0x00080000); // dim red
    neopixel_fifo_write(0x00000800); // dim green
    neopixel_fifo_write(0x00000008); // dim blue

    printf("Pixels written\n");
    uart_write_flush();

    sleep_ms(10);

    printf("Done\n");
    uart_write_flush();

    return 0;
}