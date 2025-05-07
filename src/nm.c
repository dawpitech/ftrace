/*
** EPITECH PROJECT, 2025
** nm
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

static int check_elf(int fd, Elf **elf)
{
    size_t shstrndx;

    if (fd < 0)
        return 1;
    if (elf_version(EV_CURRENT) == EV_NONE) {
        close(fd);
        return 1;
    }
    *elf = elf_begin(fd, ELF_C_READ, NULL);
    if (!(*elf)) {
        close(fd);
        return 1;
    }
    if (elf_getshdrstrndx(*elf, &shstrndx) != 0) {
        elf_end(*elf);
        close(fd);
        return 1;
    }
    return 0;
}

static void check_sym_name(const char *name, module_t *mod, GElf_Sym sym)
{
    function_t *func = NULL;

    if (name && strlen(name) > 0) {
        mod->functions = realloc(mod->functions,
            sizeof(function_t) * (mod->function_count + 1));
        func = &mod->functions[mod->function_count];
        mod->function_count++;
        func->name = strdup(name);
        func->address = (char *)mod->exec_base + sym.st_value;
    }
}

static void iterate_sym(size_t symbol_count, Elf_Data *data,
    GElf_Shdr shdr, module_t *mod)
{
    GElf_Sym sym;
    const char *name = NULL;

    for (size_t i = 0; i < symbol_count; ++i) {
        gelf_getsym(data, (int)i, &sym);
        if (GELF_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_value != 0) {
            name = elf_strptr(mod->elf, shdr.sh_link, sym.st_name);
            check_sym_name(name, mod, sym);
        }
    }
}

static void extract_functions_from_module(module_t *mod)
{
    int fd = open(mod->path, O_RDONLY);
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    Elf_Data *data = NULL;
    size_t symbol_count = 0;

    if (check_elf(fd, &mod->elf))
        return;
    scn = elf_nextscn(mod->elf, scn);
    while (scn != NULL) {
        gelf_getshdr(scn, &shdr);
        if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM) {
            data = elf_getdata(scn, NULL);
            symbol_count = shdr.sh_size / shdr.sh_entsize;
            iterate_sym(symbol_count, data, shdr, mod);
        }
        scn = elf_nextscn(mod->elf, scn);
    }
    elf_end(mod->elf);
    close(fd);
}

static void extend_module(mem_map_t *map, size_t i, parsed_string_t *parsed)
{
    module_t *mod = NULL;

    mod = &map->modules[i];
    if (parsed->start < mod->start)
        mod->start = parsed->start;
    if (parsed->end > mod->end)
        mod->end = parsed->end;
    if (!mod->exec_base && strstr(parsed->perms, "x"))
        mod->exec_base = parsed->start;
}

static void handle_module(int *found, mem_map_t *map, parsed_string_t *parsed)
{
    module_t *mod = NULL;

    *found = 1;
    map->modules = realloc(map->modules, sizeof(module_t) *
        (map->module_count + 1));
    mod = &map->modules[map->module_count];
    map->module_count++;
    strncpy(mod->path, parsed->path, PATH_MAX);
    mod->start = parsed->start;
    mod->end = parsed->end;
    mod->exec_base = (strstr(parsed->perms, "x")) ? parsed->start : NULL;
    mod->functions = NULL;
    mod->function_count = 0;
    extract_functions_from_module(mod);
}

void dispatch_mod_operation(parsed_string_t *parsed, mem_map_t *map,
    size_t i, int *found)
{
    if (*found && strcmp(parsed->path, map->modules[i].path) == 0) {
        extend_module(map, i, parsed);
        return;
    }
    if (i == map->module_count)
        handle_module(found, map, parsed);
}
