/*
** EPITECH PROJECT, 2025
** strace
** File description:
** uwu
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <string.h>
#include "../include/syscall.h"

static int iterate_args(pid_t child, syscall_t *sys, args_t *args,
    unsigned long long *args_)
{
    long arg = 0;
    int tot = 0;

    for (int i = 0; i < sys->argc; i++) {
        if (i > 0)
            tot += printf(", ");
        arg = args_[i];
        if (sys->args[i] == VOID) {
            tot += printf("?");
            continue;
        }
        tot += printf("0x%x", (unsigned int)arg);
        continue;
    }
    return tot;
}

static int print_syscall(pid_t child, struct user_regs_struct *regs,
    args_t *args)
{
    syscall_t *sys = NULL;
    int tot = 0;
    unsigned long long args_[6] = {
        regs->rdi, regs->rsi, regs->rdx,
        regs->rcx, regs->r8, regs->r9,
    };

    if (regs->rax > 330)
        return tot;
    sys = &table[regs->rax];
    tot += printf("Syscall %s(", sys->name);
    tot += iterate_args(child, sys, args, args_);
    return tot;
}

static void print_ret(args_t *args, struct user_regs_struct *regs, int tot)
{
    syscall_t *sys = NULL;

    if (regs->rax <= 330) {
        sys = &table[regs->rax];
    }
    if (sys != NULL && strcmp(sys->name, "exit_group") == 0)
        args->exit_code = regs->rdi;
    printf(")");
    if (sys != NULL && sys->args[sys->argc] == VOID) {
        printf(" = ?\n");
    } else {
        printf(" = 0x%X\n", (unsigned int)regs->rax);
    }
    fflush(stdout);
}

int maybe_print_syscall(unsigned char *instr, pid_t child,
    args_t *args, struct user_regs_struct *regs)
{
    int status = 0;
    int tot = 0;

    if (instr[0] == 0x0F && instr[1] == 0x05) {
        tot += print_syscall(child, regs, args);
        ptrace(PTRACE_SINGLESTEP, child, 0, 0);
        waitpid(child, &status, 0);
        ptrace(PTRACE_GETREGS, child, 0, regs);
        print_ret(args, regs, tot);
        if (WIFEXITED(status))
            return 1;
    }
    return 0;
}
