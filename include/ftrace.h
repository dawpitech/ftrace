/*
** EPITECH PROJECT, 2025
** strace
** File description:
** uwu
*/

#ifndef FTRACE_H
    #define FTRACE_H

    #include <linux/limits.h>
    #include <stddef.h>
    #define MAX_STRING_LEN 512
    #define EXIT_FAILURE_TECH 84
    #define MAX_CALL_DEPTH 1024
    #define NAME_BUF_SIZE PATH_MAX + 64

typedef struct syscall_s {
    int id;
    char *name;
    int argc;
    int args[6];
} syscall_t;

typedef struct args_s {
    int exit_code;
    char *command;
    char filename[PATH_MAX];
} args_t;

#endif
