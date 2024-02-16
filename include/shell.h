#ifndef SHELL_H
#define SHELL_H

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <utils.h>

// Function Declarations for builtin shell commands:
int psh_cd(char **args);
int psh_help(char **args);
int psh_pwd(char **args);
int psh_exit(char **args);

// List of builtin commands, followed by their corresponding functions.
extern char *builtin_str[];
extern int (*builtin_func[])(char **);

int psh_num_builtins();

#endif // SHELL_H