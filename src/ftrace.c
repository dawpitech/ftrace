/*
** EPITECH PROJECT, 2025
** strace
** File description:
** uwu
*/

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

#define MAX_CALL_DEPTH 1024

static void trace_func(unsigned char *instr, struct user_regs_struct *regs)
{
    static int call_stack_top = -1;
    static unsigned long call_stack[MAX_CALL_DEPTH];

    if (instr[0] == 0xE8) {
        int32_t offset;
        memcpy(&offset, &instr[1], sizeof(offset));
        unsigned long func_addr = regs->rip + 5 + offset;
        printf("Entering function 0x%lx at 0x%llx\n", func_addr, regs->rip);
        if (call_stack_top < MAX_CALL_DEPTH - 1)
            call_stack[++call_stack_top] = func_addr;
    }
    if (instr[0] == 0xC3) {
        if (call_stack_top >= 0) {
            unsigned long returning_from = call_stack[call_stack_top--];
            printf("Returning from function 0x%lx\n", returning_from);
        }
    }
}

int trace_syscalls(pid_t child, args_t *args)
{
    int status;
    struct user_regs_struct regs;
    unsigned char instr[2];
    long data = 0;

    while (1) {
        ptrace(PTRACE_GETREGS, child, 0, &regs);
        data = ptrace(PTRACE_PEEKTEXT, child, (void *)regs.rip, 0);
        instr[0] = data & 0xFF;
        instr[1] = (data >> 8) & 0xFF;
        if (maybe_print_syscall(instr, child, args, &regs))
            break;
	trace_func(instr, &regs);
        ptrace(PTRACE_SINGLESTEP, child, 0, 0);
        waitpid(child, &status, 0);
        if (WIFEXITED(status))
            break;
    }
    return args->exit_code;
}

static int create_process(char **argv, int argc, char **envp, args_t *args)
{
    pid_t pid = fork();

    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        raise(SIGSTOP);
        execve(argv[1],
            &argv[1], envp);
        return EXIT_FAILURE_TECH;
    }
    printf("+++ exited with %d +++\n", trace_syscalls(pid, args));
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
    extract_obj(args.filename, &args);
    return 0;
    return create_process(argv, argc, envp, &args);
}
