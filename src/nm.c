/*
** EPITECH PROJECT, 2025
** nm
** File description:
** uwu
*/

#include <elf.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../include/cli_args.h"
#include "../include/nm.h"

static int print_err(const char *msg)
{
    fprintf(stderr, "%s", msg);
    return 1;
}

static int is_32bit(char *data)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)data;

    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        return print_err("not a valid ELF");
    }
    if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
        return 2;
    } else if (ehdr->e_ident[EI_CLASS] == ELFCLASS64) {
        return 0;
    } else {
        return print_err("not a valid architecture");
    }
    return 0;
}

static void iterate_x64(unsigned int num_elem, char *strtab_data,
    Elf64_Sym *symbols, Elf64_Shdr *shdr)
{
    char *sym_name = NULL;

    for (unsigned int i = 0; i < num_elem; i++) {
        sym_name = (char *)(strtab_data + symbols[i].st_name);
        if (symbols[i].st_info == STT_FILE || sym_name[0] == '\0')
            continue;
        if (ELF64_ST_TYPE(symbols[i].st_info) != STT_FUNC)
            continue;
        if (symbols[i].st_value == 0)
            continue;
        printf("Function: %s at 0x%lx\n", sym_name,
            (unsigned long)symbols[i].st_value);
    }
}

static int read_file_x64(char *data, const char *filename)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)data;
    Elf64_Shdr *shdr = (Elf64_Shdr *)(data + ehdr->e_shoff);
    Elf64_Shdr *symtab = 0;
    Elf64_Shdr *strtab = 0;
    char *strtab_data = 0;
    Elf64_Sym *symbols = 0;
    unsigned int num_elem = 0;

    for (int j = 0; j < ehdr->e_shnum; j++)
        if (shdr[j].sh_type == SHT_SYMTAB)
            symtab = &shdr[j];
    if (symtab == NULL)
        return fprintf(stderr, "nm: %s: no symbols", filename);
    strtab = &shdr[symtab->sh_link];
    strtab_data = (char *)(data + strtab->sh_offset);
    symbols = (Elf64_Sym *)(data + symtab->sh_offset);
    num_elem = symtab->sh_size / sizeof(Elf64_Sym);
    iterate_x64(num_elem, strtab_data, symbols, shdr);
    free(data);
    return 0;
}

static int read_file(char *data, int fd, struct stat *st_buf, const char *file)
{
    if (data == NULL)
        return print_err("out of memory");
    if (read(fd, data, st_buf->st_size) != st_buf->st_size) {
        free(data);
        close(fd);
        return print_err("could not read the file");
    }
    if (((Elf64_Ehdr *)data)->e_shoff > (Elf64_Addr)st_buf->st_size) {
        free(data);
        close(fd);
        return fprintf(stderr,
            "nm: %s: file format not recognized\n", file);
    }
    return 0;
}

int extract_obj(const char *file, args_t *args)
{
    struct stat st_buf;
    char *data = NULL;
    int fd = open(file, O_RDONLY);

    if (fd == -1 || fstat(fd, &st_buf) != 0)
        return print_err("could not open the file");
    data = malloc(st_buf.st_size);
    if (read_file(data, fd, &st_buf, file))
        return 1;
    close(fd);
    if (is_32bit(data) == 0)
        return read_file_x64(data, file);
    return 1;
}
