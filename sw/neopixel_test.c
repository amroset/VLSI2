#include <stdint.h>
#include <stdbool.h>

#include "uart.h"
#include "print.h"
#include "gpio.h"
#include "timer.h"
#include "util.h"
#include "neopixel.h"
#include "font8x8_basic.h"

#define TB_FREQUENCY 20000000
#define TB_BAUDRATE     38400

#define WIDTH      8
#define HEIGHT     8
#define NUM_PIXELS (WIDTH * HEIGHT)
#define NUM_FRAMES 2

uint32_t nextColor(void) {
    const uint8_t scale = 8;
    static uint8_t row = 0;
    static uint8_t col = 0;
    uint8_t r = (uint8_t)(row * (256/WIDTH))/scale;
    uint8_t g = (uint8_t)(col * (256/HEIGHT))/scale;
    uint8_t b = (uint8_t)(255 - (row+col) * (256/(WIDTH+HEIGHT)))/scale;
    row++;
    if(row >= HEIGHT) { row = 0; col++; }
    if(col >= WIDTH)  { col = 0; }
    return ((r << 16) | (g << 8) | b);
}

void renderCharacter(uint32_t *frame, char character) {
    uint8_t font_bit;
    uint32_t pixel;
    uint32_t color = nextColor();
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            if (character >= 128) character = 0;
            font_bit = (font8x8_basic[character][col] >> (HEIGHT-row-1)) & 0x01;
            pixel = font_bit ? color : 0;
            frame[row*WIDTH + col] = pixel;
        }
    }
}

// Send NUM_PIXELS pixels from a pointer directly via OBI FIFO
void neopixel_send_frame(uint32_t *pixels) {
    *reg32(NPX_BASE_ADDR, NPX_FIFO_REG_OFFSET) = NPX_FIFO_REG_MODE_FIFO;
    for (int i = 0; i < NUM_PIXELS; i++) {
        *reg32(NPX_BASE_ADDR, NPX_FIFO_DATA_REG_OFFSET) = pixels[i];
    }
    *reg32(NPX_BASE_ADDR, NPX_FIFO_REG_OFFSET) = NPX_FIFO_REG_DEACTIVATED;
}

const char *hello = "Hello World!";
char string[255];

// Two frames in regular SRAM — no .dma_data needed
uint32_t frame[NUM_FRAMES * NUM_PIXELS];

int main(void) {
    uint32_t *frameOne = &frame[0];
    uint32_t *frameTwo = &frame[NUM_PIXELS];

    for (int i = 0; i < 13; i++)
        string[i] = hello[i];

    uart_init();
    neopixel_init();
    printf("Starting OBI test...\n");
    uart_write_flush();

    for (int i = 0; i < 255; i++) {
        renderCharacter(frameTwo, string[i]);
        renderCharacter(frameOne, string[i+1]);

        for (uint8_t shift = 0; shift < 8; shift++) {
            // sliding window: start shift rows before frameTwo
            // frameTwo - shift*WIDTH points into the overlap between the two frames
            neopixel_send_frame(frameTwo - shift * WIDTH);
            sleep_ms(30);
        }

        if (string[i+1] == '\n' || string[i+1] == 0)
            break;
    }

    printf("Done\n");
    uart_write_flush();

    return 0;  // signals jtag_wait_for_eoc to terminate
}