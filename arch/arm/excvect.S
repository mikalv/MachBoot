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
 * Exception handler code.
 */

#include "asm_help.h"

/**
 * bleh_undef
 *
 * Undefined instruction handler.
 */
EnterARM(bleh_undef)
    bl      _locore_halt_system

/**
 * bleh_swi
 *
 * System call handler.
 */
EnterARM(bleh_swi)
    bl      _locore_halt_system

/**
 * bleh_dataexc
 *
 * ????
 */
EnterARM(bleh_dataexc)
    bl      _locore_halt_system

/**
 * bleh_fiq
 *
 * FIQ handler.
 */
EnterARM(bleh_fiq)
    bl      _locore_halt_system

/**
 * bleh_irq
 *
 * IRQ handler.
 */
EnterARM(bleh_irq)
    bl      _locore_halt_system

/**
 * bleh_prefabt
 *
 * Prefetch abort handler.
 */
EnterARM(bleh_prefabt)
    bl      _locore_halt_system

/**
 * bleh_dataabt
 *
 * Data abort handler.
 */
EnterARM(bleh_dataabt)
    bl      _locore_halt_system

/* Exception Stacks */
.data
_excstack_begin:
    .space (4096), 0
_excstack_end:

