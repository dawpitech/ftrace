/*
** EPITECH PROJECT, 2025
** nm
** File description:
** uwu
*/

#ifndef NM_H
    #define NM_H
    #include "cli_args.h"
    #include <libelf.h>
    #define MM(x) x--

typedef struct function_s {
    char *name;
    void *address;
} function_t;

typedef struct {
    char path[PATH_MAX];
    void *start;
    void *end;
    void *exec_base;
    function_t *functions;
    size_t function_count;
    Elf *elf;
} module_t;

typedef struct parsed_string_s {
    void *start;
    void *end;
    char line[PATH_MAX + 100];
    char path[PATH_MAX];
    char perms[5];
} parsed_string_t;

typedef struct mem_map_s {
    module_t *modules;
    size_t module_count;
} mem_map_t;

void dispatch_mod_operation(parsed_string_t *parsed, mem_map_t *map,
    size_t i, int *found);
void free_memory_maps(mem_map_t *map);
mem_map_t *get_memory_maps(int pid);

#endif
