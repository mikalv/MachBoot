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
#include <stdarg.h>

/* Uart stuff */
#define HwReg(x) *((volatile unsigned long*)(x))
uint32_t gS5L8930XUartBase = 0x82500000;

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

/**
 * uart_putc
 *
 * Put a character to the system console.
 */
void uart_putchar(int c)
{
    if(c == '\n')
        uart_putchar('\r');

    /*
     * Wait for FIFO queue to empty. 
     */
    while (HwReg(gS5L8930XUartBase + UFSTAT) & UART_UFSTAT_TXFIFO_FULL)
        barrier();

    HwReg(gS5L8930XUartBase + UTXH) = c;
    return;
}

int uart_getchar(void)
{
    /*
     * Wait for a character. 
     */
    uint32_t ufstat = HwReg(gS5L8930XUartBase + UFSTAT);
    int can_read = 0;

    can_read = (ufstat & UART_UFSTAT_RXFIFO_FULL) | (ufstat & 0xF);
    if(can_read)
        return HwReg(gS5L8930XUartBase + URXH);
    else
        return -1;

    return -1;
}
