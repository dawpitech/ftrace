/*
** EPITECH PROJECT, 2025
** public_nm
** File description:
** uwu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <gelf.h>
#include "../include/nm.h"

static FILE *open_maps_file(int pid)
{
    char path[PATH_MAX] = {0};

    snprintf(path, PATH_MAX, "/proc/%i/maps", pid);
    return fopen(path, "r");
}

static int get_mod_count(mem_map_t *map, char *path)
{
    size_t i = 0;

    for (i = 0; i < map->module_count; ++i) {
        if (strcmp(map->modules[i].path, path) == 0)
            break;
    }
    return i;
}

mem_map_t *get_memory_maps(int pid)
{
    FILE *maps_file = open_maps_file(pid);
    mem_map_t *map = NULL;
    size_t i = 0;
    parsed_string_t parsed = {0};
    int found = 0;

    if (!maps_file)
        return NULL;
    map = calloc(1, sizeof(mem_map_t));
    while (fgets(parsed.line, sizeof(parsed.line), maps_file)) {
        if (sscanf(parsed.line, "%lx-%lx %4s %*s %*s %*s %[^\n]",
            (unsigned long *)&parsed.start, (unsigned long *)&parsed.end,
            parsed.perms, parsed.path) < 3)
            continue;
        i = get_mod_count(map, parsed.path);
        dispatch_mod_operation(&parsed, map, i, &found);
    }
    fclose(maps_file);
    return map;
}

void free_memory_maps(mem_map_t *map)
{
    if (!map)
        return;
    for (size_t i = 0; i < map->module_count; ++i) {
        for (size_t j = 0; j < map->modules[i].function_count; ++j) {
            free(map->modules[i].functions[j].name);
        }
        free(map->modules[i].functions);
    }
    free(map->modules);
    free(map);
}
