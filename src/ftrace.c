/*
** EPITECH PROJECT, 2025
** strace
** File description:
** uwu
*/

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include "../include/cli_args.h"
#include "../include/strace.h"
#include "../include/nm.h"
#include <string.h>
#include <unistd.h>

static const char *return_func_name(module_t *mod, unsigned long func_addr,
    unsigned long addr, char *out_buf)
{
    snprintf(out_buf, NAME_BUF_SIZE, "func_0x%lx@%s", addr, mod->path);
    return out_buf;
}

static const char *find_function_name(unsigned long addr, mem_map_t **mapptr,
    char *out_buf, pid_t pid)
{
    module_t *mod = NULL;
    unsigned long func_addr = 0;
    mem_map_t *map = *mapptr;

    for (size_t i = 0; i < map->module_count; ++i) {
        mod = &map->modules[i];
        if ((void *)addr >= mod->start && (void *)addr < mod->end)
            return return_func_name(mod, func_addr, addr, out_buf);
    }
    if (pid != 0) {
        free_memory_maps(*mapptr);
        *mapptr = get_memory_maps(pid);
        return find_function_name(addr, mapptr, out_buf, 0);
    }
    snprintf(out_buf, NAME_BUF_SIZE, "func_0x%lx", addr);
    return out_buf;
}

static void incr_stack(int *call_stack_top, unsigned long *call_stack,
    unsigned long func_addr)
{
    if (func_addr == 0)
        return;
    if (*call_stack_top < MAX_CALL_DEPTH - 1) {
        ++(*call_stack_top);
        call_stack[*call_stack_top] = func_addr;
    }
}

static int is_entering_got(module_t *mod, struct user_regs_struct *regs,
    pid_t pid, unsigned long *func_addr)
{
    unsigned long func_addr_ = 0;

    for (size_t j = 0; j < mod->function_count; ++j) {
        func_addr_ = (unsigned long)mod->functions[j].address +
            (unsigned long)mod->start;
        if (func_addr_ != regs->rip)
            continue;
        printf("Entering function %s at 0x%lx\n", mod->functions[j].name,
            ptrace(PTRACE_PEEKDATA, pid, regs->rsp, NULL));
        *func_addr = func_addr_;
        return 1;
    }
    return 0;
}

static int check_for_got(mem_map_t *map, struct user_regs_struct *regs,
    pid_t pid, unsigned long *func_addr)
{
    if (*func_addr != 0)
        return 0;
    for (size_t i = 0; i < map->module_count; ++i) {
        if (is_entering_got(&map->modules[i], regs, pid, func_addr))
            return 1;
    }
    return 0;
}

static void trace_func(unsigned char *instr, struct user_regs_struct *regs,
    mem_map_t **map, pid_t pid)
{
    static int call_stack_top = -1;
    static unsigned long call_stack[MAX_CALL_DEPTH];
    unsigned long func_addr = 0;
    char name_buf[NAME_BUF_SIZE] = {0};
    const char *func_name = NULL;
    unsigned long returning_from = 0;

    if (instr[0] == 0xC3 && call_stack_top >= 0) {
        returning_from = call_stack[MM(call_stack_top)];
        func_name = find_function_name(returning_from, map, name_buf, pid);
        printf("Leaving function %s\n", func_name);
    }
    if (instr[0] == 0xE8) {
        func_addr = regs->rip + 5 + *(int *)(instr + 1);
        func_name = find_function_name(func_addr, map, name_buf, pid);
        printf("Entering function %s at 0x%llx\n", func_name, regs->rip);
    }
    check_for_got(*map, regs, pid, &func_addr);
    incr_stack(&call_stack_top, call_stack, func_addr);
}

int trace_syscalls(pid_t child, args_t *args, mem_map_t **map, int *status)
{
    struct user_regs_struct regs;
    unsigned char instr[8];
    long data = 0;

    waitpid(child, status, 0);
    *map = get_memory_maps(child);
    ptrace(PTRACE_SETOPTIONS, child, 0LL, 64LL);
    while (1) {
        ptrace(PTRACE_GETREGS, child, 0, &regs);
        data = ptrace(PTRACE_PEEKTEXT, child, (void *)regs.rip, 0);
        memcpy(instr, &data, sizeof(long));
        if (maybe_print_syscall(instr, child, args, &regs))
            break;
        trace_func(instr, &regs, map, child);
        ptrace(PTRACE_SINGLESTEP, child, 0, 0);
        waitpid(child, status, 0);
        if (WIFEXITED(*status))
            break;
    }
    return args->exit_code;
}

static int create_process(char **argv, int argc, char **envp, args_t *args)
{
    pid_t pid = fork();
    mem_map_t *map = NULL;
    int status;

    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execve(argv[1],
            &argv[1], envp);
        return EXIT_FAILURE_TECH;
    }
    printf("Syscall execve(0x0, 0x0, 0x0) = 0x0\n");
    printf("+++ exited with %d +++\n", trace_syscalls(pid, args, &map,
        &status));
    free_memory_maps(map);
    return 0;
}

// ReSharper disable once CppJoinDeclarationAndAssignment
int main(const int argc, char **argv, char **envp)
{
    args_t args = {0};

    if (argc < 2 || strcmp(argv[1], "-help") == 0
        || strcmp(argv[1], "--help") == 0)
        return print_help(), EXIT_FAILURE_TECH;
    strcpy(args.filename, argv[1]);
    return create_process(argv, argc, envp, &args);
}
