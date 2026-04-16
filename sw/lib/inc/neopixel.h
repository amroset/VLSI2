// Copyright 2025 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Philippe Sauter <phsauter@iis.ee.ethz.ch>

#pragma once

#include <stdint.h>
#include "config.h"


// Register offsets (relative to NPX_BASE_ADDR = 0x2000_1000)
// --- Read-only info registers ---
#define NPX_MAX_NUM_PIXEL_REG_OFFSET    (0x000)   // max pixels supported (256)
#define NPX_MIN_FREQ_REG_OFFSET         (0x020)   // min allowed freq (3 MHz)
// --- Timing/config registers (writable) ---
#define NPX_NUM_PIXEL_REG_OFFSET        (0x040)
#define NPX_TIMING_T1H_REG_OFFSET       (0x060)
#define NPX_TIMING_T1L_REG_OFFSET       (0x080)
#define NPX_TIMING_T0H_REG_OFFSET       (0x0A0)
#define NPX_TIMING_T0L_REG_OFFSET       (0x0C0)
#define NPX_TIMING_LATCH_REG_OFFSET     (0x0E0)
#define NPX_TIMING_SLEEP_REG_OFFSET     (0x100)
// --- DMA registers (writable) ---
#define NPX_DMA_START_REG_OFFSET        (0x120)   // source SRAM address
#define NPX_DMA_NUM_BYTES_REG_OFFSET    (0x140)   // number of bytes to transfer
#define NPX_DMA_VALID_REG_OFFSET        (0x160)   // write 1 then 0 to arm DMA
// --- FIFO mode control (writable) ---
#define NPX_FIFO_REG_OFFSET             (0x180)
#define NPX_FIFO_REG_DEACTIVATED        (0x00)
#define NPX_FIFO_REG_MODE_FIFO          (0x01)    // CPU writes pixels via OBI (IRQ active)
#define NPX_FIFO_REG_MODE_DMA           (0x02)    // DMA feeds FIFO (IRQ disabled)
// NOTE: switching modes FLUSHES the FIFO

// --- FIFO data / status (NPX_BASE_ADDR + 0x200, in FIFO address region) ---
// Write: push 24-bit pixel color to FIFO (only in FIFO mode; backpressure stalls core when full)
// Read:  returns current FIFO fill level (0..16) — useful for polling
#define NPX_FIFO_DATA_REG_OFFSET        (0x200)

// NOTE: NPX_IRQ_MASK, NPX_FIFO_LOW_THRES, NPX_FIFO_HIGH_THRES do NOT exist in RTL.
// The FIFO interrupt is controlled solely via the mie CSR (bit 19 = 16+3).
// Thresholds are hardcoded: low=2, high=14 (in neopixel_pkg.sv).
// The FIFO interrupt is ONLY active when NPX_FIFO_REG = NPX_FIFO_REG_MODE_FIFO.

// Reasonable defaults for 20 MHz clock (25 cycles per bit)
#define NPX_NUM_PIXEL_DEFAULT           (  64)
#define NPX_TIMING_T1H_DEFAULT          (  16)
#define NPX_TIMING_T1L_DEFAULT          (   9)
#define NPX_TIMING_T0H_DEFAULT          (   8)
#define NPX_TIMING_T0L_DEFAULT          (  17)
#define NPX_TIMING_LATCH_DEFAULT        (1100)
#define NPX_TIMING_SLEEP_DEFAULT        (   1)

// Approximate frame timing at default settings (cycles at 20 MHz):
// 24 bits/pixel * 25 avg cycles/bit * num_pixels + latch
#define NPX_FRAME_CYCLES(npix)          ((npix) * 24 * 25)
#define NPX_FRAME_TOTAL_CYCLES(npix)    (NPX_FRAME_CYCLES(npix) + NPX_TIMING_LATCH_DEFAULT)

void neopixel_init(void);
void neopixel_fifo_write(uint32_t color);
void neopixel_setup_dma(uint32_t *data, uint32_t num_bytes);

// Enable FIFO-low interrupt (mie bit 19). Must be in FIFO mode for IRQ to fire.
static inline void neopixel_activate_irq(void) {
    asm volatile("csrs mie, %0" ::"r"(1u << (16+3)) : "memory");
}

// Disable FIFO interrupt (mie bit 19). Safe to call from ISR.
static inline void neopixel_deactivate_irq(void) {
    asm volatile("csrc mie, %0" ::"r"(1u << (16+3)) : "memory");
}
