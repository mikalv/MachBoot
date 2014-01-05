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
/*
 * Start of Mach Boot.
 */

#include "asm_help.h"
#include "linux_atags.h"

.arm
.code 32

.section .reset,"ax"
.align 4


#define S3C2410_URXH      (0x24)
#define S3C2410_UTXH      (0x20)
#define S3C2410_ULCON     (0x00)
#define S3C2410_UCON      (0x04)
#define S3C2410_UFCON     (0x08)
#define S3C2410_UMCON     (0x0C)
#define S3C2410_UTRSTAT   (0x10)
#define S3C2410_UERSTAT   (0x14)
#define S3C2410_UFSTAT    (0x18)
#define S3C2410_UMSTAT    (0x1C)
#define S3C2410_UBRDIV    (0x28)
#define S3C2410_UDIVSLOT  (0x2C)
#define S3C2410_UINTMSK   (0x38)

#define S3C2410_LCON_CFGMASK      ((0xF<<3)|(0x3))

#define S3C2410_LCON_CS5      (0x0)
#define S3C2410_LCON_CS6      (0x1)
#define S3C2410_LCON_CS7      (0x2)
#define S3C2410_LCON_CS8      (0x3)
#define S3C2410_LCON_CSMASK   (0x3)

#define S3C2410_LCON_PNONE    (0x0)
#define S3C2410_LCON_PEVEN    (0x5 << 3)
#define S3C2410_LCON_PODD     (0x4 << 3)
#define S3C2410_LCON_PMASK    (0x7 << 3)

#define S3C2410_LCON_STOPB    (1<<2)
#define S3C2410_LCON_IRM          (1<<6)

#define S3C2440_UCON_CLKMASK      (3<<10)
#define S3C2440_UCON_PCLK     (0<<10)
#define S3C2440_UCON_UCLK     (1<<10)
#define S3C2440_UCON_PCLK2    (2<<10)
#define S3C2440_UCON_FCLK     (3<<10)
#define S3C2443_UCON_EPLL     (3<<10)
#define S3C2410_UFCON_FIFOMODE    (1<<0)


#define S3C2410_UFSTAT_TXFULL     (1<<24)
#define S3C2410_UFSTAT_RXFULL     (1<<8)
#define S3C2410_UFSTAT_TXMASK     (255<<16)
#define S3C2410_UFSTAT_TXSHIFT    (16)
#define S3C2410_UFSTAT_RXMASK     (255<<0)
#define S3C2410_UFSTAT_RXSHIFT    (0)

/* UFSTAT S3C2443 same as S3C2440 */
#define S3C2440_UFSTAT_TXFULL     (1<<24)
#define S3C2440_UFSTAT_RXFULL     (1<<8)
#define S3C2440_UFSTAT_TXSHIFT    (16)
#define S3C2440_UFSTAT_RXSHIFT    (0)
#define S3C2440_UFSTAT_TXMASK     (255<<16)
#define S3C2440_UFSTAT_RXMASK     (255)

#define S3C2410_UTRSTAT_TXE   (1<<2)
#define S3C2410_UTRSTAT_TXFE      (1<<1)
#define S3C2410_UTRSTAT_RXDR      (1<<0)

#define S3C2410_UERSTAT_OVERRUN   (1<<0)
#define S3C2410_UERSTAT_FRAME     (1<<2)
#define S3C2410_UERSTAT_BREAK     (1<<3)
#define S3C2443_UERSTAT_PARITY    (1<<1)


/* The S5PV210/S5PC110 implementations are as belows. */

    .macro fifo_level_s5pv210 rd, rx
        ldr \rd, [\rx, # S3C2410_UFSTAT]
        and \rd, \rd, #S5PV210_UFSTAT_TXMASK
    .endm

    .macro  fifo_full_s5pv210 rd, rx
        ldr \rd, [\rx, # S3C2410_UFSTAT]
        tst \rd, #S5PV210_UFSTAT_TXFULL
    .endm

/* The S3C2440 implementations are used by default as they are the
 * most widely re-used */

    .macro fifo_level_s3c2440 rd, rx
        ldr \rd, [\rx, # S3C2410_UFSTAT]
        and \rd, \rd, #S3C2440_UFSTAT_TXMASK
    .endm

#ifndef fifo_level
#define fifo_level fifo_level_s3c2440
#endif

    .macro  fifo_full_s3c2440 rd, rx
        ldr \rd, [\rx, # S3C2410_UFSTAT]
        tst \rd, #S3C2440_UFSTAT_TXFULL
    .endm

#ifndef fifo_full
#define fifo_full fifo_full_s3c2440
#endif
    
    .macro addruart, rx
        ldr     \rx, =0x82500000
    .endm

    .macro  senduart,rd,rx
        strb    \rd, [\rx, # S3C2410_UTXH]
    .endm

    .macro  busyuart, rd, rx
        ldr \rd, [\rx, # S3C2410_UFCON]
        tst \rd, #S3C2410_UFCON_FIFOMODE    @ fifo enabled?
        beq 1001f               @
        @ FIFO enabled...
1003:
        fifo_full \rd, \rx
        bne 1003b
        b   1002f

1001:
        @ busy waiting for non fifo
        ldr \rd, [\rx, # S3C2410_UTRSTAT]
        tst \rd, #S3C2410_UTRSTAT_TXFE
        beq 1001b

1002:       @ exit busyuart
    .endm

    .macro  waituart,rd,rx
        ldr \rd, [\rx, # S3C2410_UFCON]
        tst \rd, #S3C2410_UFCON_FIFOMODE    @ fifo enabled?
        beq 1001f               @
        @ FIFO enabled...
1003:
        fifo_level \rd, \rx
        teq \rd, #0
        bne 1003b
        b   1002f
1001:
        @ idle waiting for non fifo
        ldr \rd, [\rx, # S3C2410_UTRSTAT]
        tst \rd, #S3C2410_UTRSTAT_TXFE
        beq 1001b

1002:       @ exit busyuart
    .endm

/**
 * ARM Vector Table
 *
 * The system will map this to 0x0 during platform initialization and relocate
 * and restart the image if necessary.
 */

/*
 * I honestly wish llvm supported the "ldr rX, =var" syntax.
 */
.globl  _ExceptionVectorsBase
_ExceptionVectorsBase:
    b       _start              // reset
    ldr     pc, [pc, #24]       // undef
    ldr     pc, [pc, #24]       // swi
    ldr     pc, [pc, #24]       // prefetch
    ldr     pc, [pc, #24]       // data abort
    ldr     pc, [pc, #24]       // dataexc
    ldr     pc, [pc, #24]       // irq
    ldr     pc, [pc, #24]       // fiq

_vectorTable:
    .long   _start
    .long   _bleh_undef
    .long   _bleh_swi
    .long   _bleh_prefabt
    .long   _bleh_dataabt
    .long   _bleh_dataexc
    .long   _bleh_irq
    .long   _bleh_fiq

/**
 * The point of this bootstrapper is to copy the kernel to the proper
 * physical address and start it.
 *
 * The known states of the registers are:
 *  r0 - #0
 *  r1 - Machine type
 *  r2 - PA of ATAGs list
 */
EnterARM(start)
    /* We are in supervisor, no interrupts. */
    cpsid   if, #0x13

    /* Check sure we're at the right load address */
    ldr    r4, =_ExceptionVectorsBase
    adr    r5, _ExceptionVectorsBase
    cmp    r4, r5
    beq    L_relocated

    /*
     * We should not trash the r0-r2 registers, but we don't guarantee that we won't
     * overwrite the ATAGs.... (We probably won't anyway.)
     */

    mov    r6, #(1 * 1024 * 1024)	// HACK
L_relocate_loop:
    ldr    r8, [r5], #4
    str    r8, [r4], #4
    subs   r6, r6, #4
    bgt    L_relocate_loop

    /* We're done relocating, jump back into the image. */
    ldr    r4, =_ExceptionVectorsBase
    bx     r4

L_relocated:
    /*
     * Things look semi good, (we hope we're running at the right
     * text base, this is assumed.)
    */
    ldr     sp, =_tempstack_end
    mov     r7, #0

    /*
     * Go to core bootstrap now.
     */
    bl      corestart_main

lol_no_atags:
    /* lol no atags */
    bl      _failure

#define ENTRY(name) \
    .globl name; \
    .align 4 ; \
    name: ;
#define ENDPROC(name)

/*
 * Useful debugging routines
 */
ENTRY(printhex8)
        mov r1, #8
        b   printhex
ENDPROC(printhex8)

ENTRY(printhex4)
        mov r1, #4
        b   printhex
ENDPROC(printhex4)

ENTRY(printhex2)
        mov r1, #2
printhex:   adr r2, hexbuf
        add r3, r2, r1
        mov r1, #0
        strb    r1, [r3]
1:      and r1, r0, #15
        mov r0, r0, lsr #4
        cmp r1, #10
        addlt   r1, r1, #'0'
        addge   r1, r1, #'a' - 10
        strb    r1, [r3, #-1]!
        teq r3, r2
        bne 1b
        mov r0, r2
        b   printascii
ENDPROC(printhex2)

ENTRY(printascii)
        addruart r3
        b   2f
1:      waituart r2, r3
        senduart r1, r3
        busyuart r2, r3
        teq r1, #'\n'
        moveq   r1, #'\r'
        beq 1b
2:      teq r0, #0
        ldrneb  r1, [r0], #1
        teqne   r1, #0
        bne 1b
        mov pc, lr
ENDPROC(printascii)


hexbuf:     .space 16

.text

/**
 * failure
 *
 * This function is called when we fail core initialization. :(
 */
EnterARM(failure)
EnterARM(locore_halt_system)
    cpsid   if
    b       .

/**
 * locore_jump_to
 *
 * Shut down the bootloader and start the new OS image.
 */
EnterARM(locore_jump_to)
    /* Disable interrupts */
    cpsid   if, #0x13

    /* Disable L2 cache */
    mov     r9, r0
    mov     r10, r1

    /* Disable MMU */
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #1
    mcr     p15, 0, r0, c1, c0, 0

    /* Set TTBR0/1 to zero. */
    mov     r0, #0
    mcr     p15, 0, r0, c2, c0, 0
    mcr     p15, 0, r0, c2, c0, 1

    /* Clear TLB */
    mcr     p15, 0, r0, c8, c7, 0
    dsb     ish
    isb     sy

    mrc     p15, 0, r0, c1, c0, 1   /* read Auxiliary Control Register */
    bic     r0, r0, #0x00000002     /* disable L2 cache */
    mcr     p15, 0, r0, c1, c0, 1   /* store Auxiliary Control Register */

    /* Disable caching entirely. */
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #0x00002300 
    bic     r0, r0, #0x00000005
    bic     r0, r0, #0x00001000
    bic     r0, r0, #(1 << 2)
    mcr     p15, 0, r0, c1, c0, 0

    /* Disable VFP/SIMD */
    mov     r0, #0x00000000
    mcr     p10, #0x7, r0, c8, c0, #0

    /* Disable I-cache */
    mrc     p15, 0, r0, c1, c0, 2
    bic     r0, r0, #0x00f00000
    mcr     p15, 0, r0, c1, c0, 2

    /* Clear caches. */
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4

    /* Disable MMU */
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #1
    mcr     p15, 0, r0, c1, c0, 0

    /* Clear prefetch buffer */
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4

    isb     sy
    dsb     sy

    /* Point of no return */
    mov     lr, r9
    mov     r0, r1
    bx      lr  
  
deadloop:
    b       .


/**
 * locore_jump_to_fast
 *
 * Shut down the bootloader and start the new OS image. (no messing with anything)
 */
EnterARM(locore_jump_to_fast)
    /* Disable interrupts */
    cpsid   if, #0x13

    mov     r9, r0
    mov     lr, r9

    mov     r0, r1
    bx      lr  
  
deadloop2:
    b       .

/**
 * cpu_number.
 *
 * Get MPIDR/current CPU number (we're UP only, so who cares right now.)
 */
.globl cpu_number
cpu_number:
    mrc     p15, 0, r0, c0, c0, 5
    ands    r0, r0, #0xFF
    bx      lr

.data
.align 4
/*
 * Temporary stack.
 */
_tempstack_begin:
    .space (4096), 0
_tempstack_end:

