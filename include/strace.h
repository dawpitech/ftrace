/*
** EPITECH PROJECT, 2025
** strace
** File description:
** uwu
*/

#ifndef STRACE_H
    #define STRACE_H
    #include <unistd.h>
    #include "syscall.h"

int maybe_print_syscall(unsigned char *instr, pid_t child,
    args_t *args, struct user_regs_struct *regs);

#endif
