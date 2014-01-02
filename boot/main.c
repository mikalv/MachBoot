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

boot_args gBootArgs;
uint32_t ramdisk_base = 0x0;
uint32_t ramdisk_size = 0x0;
void* gFSLoadAddress = (void*)LOADADDR;
UInt32 kLoadSize = 16777216;

nvram_variable_list_t *gNvramVariables;

/**
 * populate_memory_info
 *
 * Populate physical memory region information.
 */
static int is_first_region = 0;
static int is_malloc_inited = 0;
#define MALLOC_SIZE          (24 * 1024 * 1024)
#define MALLOC_SIZE_SAFE     (16 * 1024 * 1024)
static void populate_memory_info(struct atag *atags)
{
    /* Only the first memory region will work for now. */
    if (is_first_region == 1)
        return;

    is_first_region = 1;

    gBootArgs.physBase = atags->u.mem.start;
    gBootArgs.memSize = atags->u.mem.size;

#if 1
    printf("initializing malloc region at 0x%08x of size %d\n",
          (char *)atags->u.mem.start + atags->u.mem.size -
          MALLOC_SIZE, MALLOC_SIZE_SAFE);
#endif

    is_malloc_inited = 1;
}

/**
 * populate_commandline_info
 *
 * Populate physical memory region information.
 */
static void populate_commandline_info(struct atag *atags)
{
    /* More than one character please. */
    if (strlen(atags->u.cmdline.cmdline) == 0)
        return;

    /* Copy it now. */
    strncpy(gBootArgs.commandLine, atags->u.cmdline.cmdline, BOOT_LINE_LENGTH);
}

/**
 * populate_ramdisk_info
 *
 * Populate initrd information.
 */
static int is_first_ramdisk = 0;
static void populate_ramdisk_info(struct atag *atags)
{
    /* Only one ramdisk allowed. */
    if (is_first_ramdisk == 1)
        return;

    is_first_ramdisk = 1;

    ramdisk_base = atags->u.initrd2.start;
    ramdisk_size = atags->u.initrd2.size;
}

extern void *_end;
extern void *_start;
extern void *_ExceptionVectorsBase, *_ttb_area;
extern const char *_bl_title, *_bl_build, *_bl_revision;

/**
 * delay_boot
 */
static bool delay_boot(void)
{
    uint32_t delay;
    nvram_variable_t var;

    var = nvram_read_variable_info(gNvramVariables, "bootdelay");

    delay = strtoul(var.setting, NULL, 0);
    
    printf("Delaying boot for %d seconds. Hit enter to break into the command prompt.\n", delay);

    while(delay--) {
        ;   /* Todo. */
    }

    return 0;
}


/**
 * machboot_main
 *
 * Actual main bootloader thread.
 */
void machboot_main(struct atag *atags)
{
    /* High level subsystem init. */
    permissions_init();

    /*
     * Announce ourselves.
     */
    printf("=======================================\n"
           "::\n"
           ":: %s\n"
           "::\n"
           "::\tBUILD_TAG: %s\n"
           "::\n"
           "::\tBUILD_STYLE: %s\n"
           "::\n"
           "::\tCOMPILE_DATE: " __DATE__ " " __TIME__ "\n"
           "::\n"
           "=======================================\n", &_bl_title, &_bl_revision, &_bl_build);

    bzero((void *)&gBootArgs, sizeof(boot_args));

    /*
     * Set up boot_args based on atag data, the rest will be filled out during
     * initialization.
     */
    struct atag_header *atag_base = (struct atag_header *)atags;
    uint32_t tag = atag_base->tag;

    while (tag != ATAG_NONE) {
        tag = atag_base->tag;

        switch (tag) {
        case ATAG_MEM:
            populate_memory_info((struct atag *)atag_base);
            break;
        case ATAG_CMDLINE:
            populate_commandline_info((struct atag *)atag_base);
            break;
        case ATAG_INITRD2:
            populate_ramdisk_info((struct atag *)atag_base);
            break;
        default:
            break;
        }

        atag_base =
            (struct atag_header *)((uint32_t *) atag_base + (atag_base->size));
    };

    /* Command prompt. */
    command_prompt();

    /* Start it. */
    panic("Nothing to do...\n");
    return;
}

/**
 * corestart_main
 *
 * Prepare the system for Darwin kernel execution.
 */
void corestart_main(uint32_t __unused, uint32_t machine_type, struct atag *atags)
{
    /* We're in. */
    init_debug();

    memset((void*)0x5f700000, 0xff, 640 * 960 * 2);

    /* Platform init. */
    init_platform();
    
    /* #We r of #helo. */
    printf("machboot start\n");

#if 0    
    /* MMU start */
    mmu_init(DRAM_BASE, DRAM_SIZE, (uint32_t)&_ExceptionVectorsBase, (uint32_t)&_ttb_area);

    /* Malloc init. */
    malloc_init((char *)DRAM_BASE + DRAM_SIZE - MALLOC_SIZE, MALLOC_SIZE_SAFE);

    /* Thread init. */
    thread_init();

    /*
     * Verify machine type.
     */
    if (machine_type != MACH_TYPE_REALVIEW_PBA8) {
        printf("********************************\n"
               "*                              *\n"
               "*  This unit is not supported  *\n"
               "*                              *\n"
               "********************************\n");
        printf("Machine type is %d, expected %d\n", machine_type,
               MACH_TYPE_REALVIEW_PBA8);
        _locore_halt_system();
    }

    /* Start the bootloader thread. */
    thread_start(arm_thread_create("bootloader", 'main'), (void*)&machboot_main, (void*)atags);

    /* Go into the idle loop. */
    _Processor_idle();
#endif
    machboot_main(NULL);

    panic("Corestart shouldn't return.\n");
}

