// Copyright 2022 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Nils Wistoff <nwistoff@iis.ee.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>

#pragma once

#define PULPINO_BASE_ADDR             0x10000000

/** SOC PERIPHERALS */
#define SOC_PERIPHERALS_BASE_ADDR     ( PULPINO_BASE_ADDR + 0xA100000 )

// Address map
#define SOCCTRL_BASE_ADDR 0x03000000
#define UART_BASE_ADDR 0x03002000
#define GPIO_BASE_ADDR 0x03005000
#define OBI_TIMER_BASE_ADDR 0x0300A000
#define CLINT_BASE_ADDR 0x02040000
#define NPX_BASE_ADDR 0x20001000
#define NPX_IRQ_MASK_REG_OFFSET 0x00000010
#define TIMER_BASE_ADDR          ( SOC_PERIPHERALS_BASE_ADDR + 0x3000 )

// Frequencies
#define TB_FREQUENCY 20000000
#define TB_BAUDRATE 115200



// Peripheral configs
// UART
#define UART_BYTE_ALIGN 4
#define UART_FREQ TB_FREQUENCY
#define UART_BAUD TB_BAUDRATE

// Interrupts
#define IRQ_SOFTWARE 3
#define IRQ_TIMER 7
#define IRQ_EXTERNAL 11
#define IRQ_OBI_TIMER 16
#define IRQ_UART 17
#define IRQ_GPIO 18
