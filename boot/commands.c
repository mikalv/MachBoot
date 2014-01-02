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

nvram_variable_list_t *gNvramVariables;

int command_help(int argc, char* argv[]) {
    command_dispatch_t *current = &gDispatch[0];
    while(current->name != NULL) {
        printf("        %-16.16s %s\n", current->name, current->description);
        current++;
    }
    return 0;
}

int command_halt(int argc, char* argv[])
{
    _locore_halt_system();
    return 0;   /* Not reached. */
}

int command_go(int argc, char* argv[])
{
    uint32_t addr = 0;

    if(argc != 1) {
        printf("usage: %s <address>\n", argv[0]);
        return -1;
    }

    addr = strtoul(argv[1], NULL, 16);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    printf("jumping into image at 0x%08x\n", addr);

    _locore_jump_to((void (**)(void *, uint32_t))addr, 0);

    return 0;
}

/* Memory read/write. */
int command_mws(int argc, char* argv[])
{
    uint32_t addr = 0;
    if(argc != 2) {
        printf("usage: %s [address] [data]\n", argv[0]);
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    strcpy((char*)addr, argv[2]);
    return 0;
}

int command_mwb(int argc, char* argv[])
{
    uint32_t addr = 0;
    uint8_t data = 0;
    if(argc != 2) {
        printf("usage: %s [address] [data]\n", argv[0]);
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    data = (uint8_t)strtoul(argv[2], NULL, 16);
    *((uint32_t*)addr) = data;
    return 0;
}

int command_mwh(int argc, char* argv[])
{
    uint32_t addr = 0;
    uint16_t data = 0;
    if(argc != 2) {
        printf("usage: %s [address] [data]\n", argv[0]);
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    data = (uint16_t)strtoul(argv[2], NULL, 16);
    *((uint32_t*)addr) = data;
    return 0;
}

int command_mw(int argc, char* argv[])
{
    uint32_t addr = 0;
    uint32_t data = 0;
    if(argc != 2) {
        printf("usage: %s [address] [data]\n", argv[0]);
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    data = (uint32_t)strtoul(argv[2], NULL, 16);
    *((uint32_t*)addr) = data;
    return 0;
}

int command_mdb(int argc, char* argv[])
{
    uint32_t addr = 0;
    if(argc != 1) {
        printf("usage: %s [address]\n", argv[0]);
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    printf("*0x%08x = 0x%02x\n", addr, *((uint8_t*)addr));
    return 0;
}

int command_mdh(int argc, char* argv[])
{
    uint32_t addr = 0;
    if(argc != 1) {
        printf("usage: %s [address]\n", argv[0]);
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    printf("*0x%08x = 0x%04x\n", addr, *((uint16_t*)addr));
    return 0;
}

int command_md(int argc, char* argv[])
{
    uint32_t addr = 0;
    if(argc != 1) {
        printf("usage: %s [address]\n", argv[0]);
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    printf("*0x%08x = 0x%08x\n", addr, *((uint32_t*)addr));
    return 0;
}

/* Hashing Memory Functions. */
int command_crc(int argc, char* argv[])
{
    uint32_t addr = 0, len = 0;
    if(argc != 2) {
        printf("not enough arguments\n"
               "usage: crc <address> <len>\n");
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);
    len = strtoul(argv[2], NULL, 10);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    printf("CRC32(0x%08x, %d) = 0x%08x\n", addr, len, crc32(0, (void*)addr, len));
    return 0;
}

int command_sha1(int argc, char* argv[])
{
    SHA1_CTX ctx;
    unsigned char hash[20];
    int i;
    uint32_t addr = 0, len = 0;

    if(argc != 2) {
        printf("not enough arguments\n"
               "usage: sha1 <address> <len>\n");
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);
    len = strtoul(argv[2], NULL, 10);

    if(!permissions_range_check(addr)) {
        printf("Permission Denied\n");
        return -1;
    }

    SHA1Init(&ctx);
    SHA1Update(&ctx, (uint8_t*)(addr), len);
    SHA1Final(hash, &ctx);

    printf("SHA1(0x%08x, %d) = ", addr, len);
    for(i=0;i<20;i++)
        printf("%02x", hash[i]);
    printf("\n");
    return 0;
}


void hexdump(unsigned char* buf, unsigned int len) {
        int i, j;
        printf("0x%08x: ", buf);
        for (i = 0; i < len; i++) {
                if (i % 16 == 0 && i != 0) {
                        for (j=i-16; j < i; j++) {
                                unsigned char car = buf[j];
                                if (car < 0x20 || car > 0x7f) car = '.';
                                printf("%c", car);
                        }
                        printf("\n0x%08x: ", buf+i);
                }
                printf("%02x ", buf[i]);
        }

        int done = (i % 16);
        int remains = 16 - done;
        if (done > 0) {
                for (j = 0; j < remains; j++) {
                        printf("   ");
                }
        }

        if ((i - done) >= 0) {
                if (done == 0 && i > 0) done = 16;
                for (j = (i - done); j < i; j++) {
                        unsigned char car = buf[j];
                        if (car < 0x20 || car > 0x7f) car = '.';
                        printf("%c", car);
                }
        }

        printf("\n\n");
}

int command_hexdump(int argc, char* argv[]) {
    uint32_t addr = 0, len = 0;
    if(argc != 2) {
        printf("not enough arguments\n"
               "usage: hexdump <address> <len>\n");
        return -1;
    }
    addr = strtoul(argv[1], NULL, 16);
    len = strtoul(argv[2], NULL, 10);

        hexdump((unsigned char*)addr, len);

        return 0;
}

/* Bootx. */
void *gKernelImage = NULL, *gDeviceTreeImage = NULL;
uint32_t gKernelSize;

/* Command dispatch. */
command_dispatch_t gDispatch[] = {
    {"help", command_help, "this list"},
    {"go", command_go, "jump directly to address"},
    {"halt", command_halt, "halt the system (good for JTAG)"},
    {"mws", command_mws, "memory write - string"},
    {"mwb", command_mwb, "memory write - 8bit"},
    {"mwh", command_mwh, "memory write - 16bit"},
    {"mw", command_mw, "memory write - 32bit"},
    {"mdb", command_mdb, "memory display - 8bit"},
    {"mdh", command_mdh, "memory display - 16bit"},
    {"md", command_md, "memory display - 32bit"},
    {"sha1", command_sha1, "SHA-1 hash of memory"},
    {"crc", command_crc, "POSIX 1003.2 checksum of memory"},
    {"hexdump", command_hexdump, "hex dump of memory"},
    {NULL, NULL, NULL},
};

