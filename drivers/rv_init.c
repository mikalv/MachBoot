/*
 * Copyright 2013, winocm. <winocm@icloud.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 *   If you are going to use this software in any form that does not involve
 *   releasing the source to this project or improving it, let me know beforehand.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "genboot.h"

static uint32_t gS5L8930XUartBase = 0x82500000;
static uint32_t gS5L8930XClockGateBase = 0xBF101000;

#define CLOCK_HZ            24000000

#define CLK_REG_OFF         0x10

#define HwReg(x) *((volatile unsigned long*)(x))

#define UART_CLOCKGATE                      0x30
#define UART_CLOCK_SELECTION_MASK         (0x3 << 10)
#define UART_CLOCK_SELECTION_SHIFT         10
#define UART_DIVVAL_MASK                         0x0000FFFF
#define UART_SAMPLERATE_MASK                 0x00030000
#define UART_SAMPLERATE_SHIFT                 16
#define UART_UCON_RXMODE_SHIFT                 0
#define UART_UCON_TXMODE_SHIFT                 2
#define UART_8BITS                                         3
#define UART_FIFO_RESET_TX                         4
#define UART_FIFO_RESET_RX                         2
#define UART_FIFO_ENABLE                         1

#define UART_UCON_MODE_IRQORPOLL         1

#define ULCON      0x0000 /* Line Control             */
#define UCON       0x0004 /* Control                  */
#define UFCON      0x0008 /* FIFO Control             */
#define UMCON      0x000C /* Modem Control            */
#define UTRSTAT    0x0010 /* Tx/Rx Status             */
#define UERSTAT    0x0014 /* UART Error Status        */
#define UFSTAT     0x0018 /* FIFO Status              */
#define UMSTAT     0x001C /* Modem Status             */
#define UTXH       0x0020 /* Transmit Buffer          */
#define URXH       0x0024 /* Receive Buffer           */
#define UBRDIV     0x0028 /* Baud Rate Divisor        */
#define UFRACVAL   0x002C /* Divisor Fractional Value */
#define UINTP      0x0030 /* Interrupt Pending        */
#define UINTSP     0x0034 /* Interrupt Source Pending */
#define UINTM      0x0038 /* Interrupt Mask           */

#define UART_UFSTAT_TXFIFO_FULL                        (0x1 << 9)
#define UART_UFSTAT_RXFIFO_FULL                        (0x1 << 8)
#define UART_UTRSTAT_TRANSMITTEREMPTY         0x4
#define UART_UMSTAT_CTS                                 0x1

#define barrier()               __asm__ __volatile__("": : :"memory");

static void s5l8930x_clock_gate_switch(int gate, int state)
{
    uint32_t __register;

    assert(gS5L8930XClockGateBase);

    if (gate > 0x3f)
        return;

    __register = CLK_REG_OFF + (gate << 2);

#if defined(BOARD_CONFIG_S5L8920X) || defined(BOARD_CONFIG_S5L8922X)
    __register -= CLK_REG_OFF;
    __register += 0x78;
#endif

    if (state) {
        HwReg(gS5L8930XClockGateBase + __register) = HwReg(gS5L8930XClockGateBase + __register) | 0xF;
    } else {
        HwReg(gS5L8930XClockGateBase + __register) = HwReg(gS5L8930XClockGateBase + __register) & ~0xF;
    }

    /*
     * Wait for the state change to take effect. 
     */
    while ((HwReg(gS5L8930XClockGateBase + __register) & 0xF) != ((HwReg(gS5L8930XClockGateBase + __register) >> 4) & 0xF))
        barrier();

    return;
}

/**
 * init_platform
 *
 * Do platform stuff for this board config (ARM Realview)
 */
void init_platform(void)
{
    /*
     * Enable clock gate. 
     */
    s5l8930x_clock_gate_switch(UART_CLOCKGATE, 1);

    /*
     * Set 8-bit frames. 
     */
    HwReg(gS5L8930XUartBase + ULCON) = UART_8BITS;

    /*
     * Use polling for RX/TX. 
     */
    HwReg(gS5L8930XUartBase + UCON) = ((UART_UCON_MODE_IRQORPOLL << UART_UCON_RXMODE_SHIFT) | (UART_UCON_MODE_IRQORPOLL << UART_UCON_TXMODE_SHIFT));

    /*
     * Set clock. 
     */
    HwReg(gS5L8930XUartBase + UCON) = (HwReg(gS5L8930XUartBase + UCON) & (~UART_CLOCK_SELECTION_MASK)) | (1 << UART_CLOCK_SELECTION_SHIFT);

    /*
     * Set baud to 115200. 
     */
    uint32_t divisorValue = CLOCK_HZ / (115200 * 16) - 1;

    HwReg(gS5L8930XUartBase + UBRDIV) = (HwReg(gS5L8930XUartBase + UBRDIV) & (~UART_DIVVAL_MASK)) | divisorValue;

    /*
     * Reset FIFO 
     */
    HwReg(gS5L8930XUartBase + UFCON) = UART_FIFO_RESET_RX | UART_FIFO_RESET_TX;

    /*
     * Enable FIFO 
     */
    HwReg(gS5L8930XUartBase + UFCON) = UART_FIFO_ENABLE;
}
