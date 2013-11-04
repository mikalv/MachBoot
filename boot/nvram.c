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

#define xstr(s) #s
#define str(s) xstr(s)

nvram_variable_list_t *gNvramVariables;

nvram_variable_t gNvramDefaultVariables[] = {
    {"build-style",         str(BUILD_STYLE), 0},
    {"build-version",       str(BUILD_TAG), 0},
    {"config_board",        str(BUILD_PLATFORM), 0},
    {"boot-args",           "-v", 0},
    {"loadaddr",            str(DRAM_BASE), 0},
    {"bootdelay",           "0", 0},
    {"auto-boot",           "false", 0},
    {"idle-off",            "true", 0},
    {"boot-device",         "nand0a", 0},
    {"boot-partition",      "0", 0},
    {"boot-path",           "/System/Library/Caches/com.apple.kernelcaches/kernelcache", 0},
    {"display-color-space", "RGB888", 0},
    {"display-timing",      str(BUILD_PLATFORM), 0},
    {"framebuffer",         str(FRAMEBUFFER_ADDRESS), 0},
    {"secure-boot",         "0x0", 0},
    {"filesize", "0x0", 0},
};

/**
 * nvram_initialize_list 
 *
 * Initialize linked list to contain nvram variable data.
 */
nvram_variable_list_t *nvram_initialize_list(void)
{
        nvram_variable_list_t *list = malloc(sizeof(nvram_variable_list_t));
        list->head = NULL;
        list->tail = &list->head;

        return list;
}

/**
 * nvram_init
 *
 * Populate nvram with initial values.
 */
int nvram_init(nvram_variable_t vars[], size_t size) {
    int i;

    gNvramVariables = nvram_initialize_list();

    for (i = 0; i <= size; i++) {
            nvram_variable_set(gNvramVariables, vars[i].name, vars[i].setting);
    }

    return 0;
}

/**
 * nvram_create_node
 *
 * Create a node to be inserted into linked list.
 */
nvram_variable_node_t *nvram_create_node(const char *name, const char *setting, int overridden)
{
        nvram_variable_node_t *node = malloc(sizeof(nvram_variable_node_t));

        node->next = NULL;
        node->value.name = strdup(name);
        node->value.setting = strdup(setting);
        node->value.overridden = overridden;

        return node;
}

/**
 * nvram_append_node
 *
 * Append a node to the end of the linked list.
 */
void nvram_append_node(nvram_variable_list_t *list, nvram_variable_node_t *node)
{
        *list->tail = node;
        list->tail = &node->next;
        node->next = NULL;
}

/**
 * nvram_remove_node
 *
 * Remove a node from the linked list.
 */
void nvram_remove_node(nvram_variable_list_t *list, nvram_variable_node_t *node)
{
        nvram_variable_node_t *current;
        nvram_variable_node_t **next = &list->head;

        while ((current = *next) != NULL) {
                if (current == node) {
                        *next = node->next;
                        if (list->tail == &node->next)
                                list->tail = next;

                        node->next = NULL;
                        break;
                }
                next = &current->next;
        }
}

/**
 * nvram_variable_set
 *
 * Add/override an vnram variable.
 */
void nvram_variable_set(nvram_variable_list_t *list, const char *name, const char *setting)
{
        nvram_variable_node_t *node;

        nvram_variable_node_t *current = list->head;

        while (current != NULL) {
                if (strcmp(current->value.name, name) == 0) {
                        current->value.name = strdup(name);
                        current->value.setting = strdup(setting);
                        current->value.overridden = 1;

                        return;
                }
                current = current->next;
        }

        node = nvram_create_node(name, setting, 0);
        nvram_append_node(list, node);
}

/**
 * nvram_variable_unset
 *
 * Unset/erase an nvram variable.
 */
int nvram_variable_unset(nvram_variable_list_t *list, const char *name)
{
        nvram_variable_node_t *current = list->head;

        while (current != NULL) {
                if (strcmp(current->value.name, name) == 0) {
                        nvram_remove_node(list, current);
                        return 0;
                }

                current = current->next;
        }

        return -1;
}

/**
 * nvram_read_variable_info
 *
 * Retrieve information about an nvram variable (such as it's value or if it's been modified).
 */
nvram_variable_t nvram_read_variable_info(nvram_variable_list_t *list, const char *name)
{
        nvram_variable_t value;

        nvram_variable_node_t *current = list->head;

        while (current != NULL) {
                if (strcmp(current->value.name, name) == 0)
                    value = current->value;
                current = current->next;
        }

        return value;
}

/**
 * nvram_dump_list
 *
 * Dump a list of all variables in nvram and their associated values and states.
 */
void nvram_dump_list(nvram_variable_list_t *list)
{
        nvram_variable_node_t *current = list->head;

        while (current != NULL) {
                printf("%s %s = %s\n", (current->value.overridden ? "P" : ""), current->value.name, current->value.setting);
                current = current->next;
        }
}

int command_setenv(int argc, char* argv[]) {
    if(argc != 2) {
        printf("usage: setenv <var> <string>\n");
        return -1;
    }

    nvram_variable_set(gNvramVariables, argv[1], argv[2]);

    return 0;
}

int command_getenv(int argc, char* argv[]) {
    nvram_variable_t var;

    if(argc != 1) {
        printf("usage: getenv <var>\n");
        return -1;
    }

    var = nvram_read_variable_info(gNvramVariables, argv[1]);

    if (var.name != NULL) {
        printf("%s\n", var.setting);
        return 0;
    } else {
        printf("no such variable: %s\n", argv[1]);
        return -1;
    }
}

int command_printenv(int argc, char* argv[]) {
    nvram_variable_t var;

    var = nvram_read_variable_info(gNvramVariables, argv[1]);

    if(argv[1]) {
        if (var.name != NULL) {
            printf("%s = '%s'\n", var.name, var.setting);
            return 0;
        } else {
            printf("no such variable: %s\n", argv[1]);
            return -1;
        }
    } else {
        nvram_dump_list(gNvramVariables);
    }

    return 0;
}
