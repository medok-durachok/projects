#ifndef SHELLLIB_H_INCLUDED
#define SHELLLIB_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define ADD_M 20

char *keyboard_enter(void);

char *file_enter(FILE *f);

void mem_all(char *word, int l_c, char **w_a, int *w_c);

char **parse(char **w_arr, char *s, int *counter, int *current_args, short *pp, short *dd);

void output(char **arr, int size);

void parse_exec(char **words_arr, char *s, int *count);

int cd(char **argv, int argc);

void redirection(char **argv, int argc, short redir_in, short redir_out);

void cmd_exec(char **sub_arr, int argc, pid_t pgid, short is_back);

int pipeline(char **argv, int argc);

int status_analysis(char **argv, int argc);

int command_exec(char **argv, int argc, short is_redirection_in, short is_redirection_out, short is_pipe);

int find_sym(char ** arr, int n, char *c);

int count_sym(char **arr, int n, char *c);

char **sub_create(char **arr, int i1, int i2);

#endif
