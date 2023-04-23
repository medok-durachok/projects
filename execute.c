#include "shelllib.h"

int cd(char **argv, int argc) {                                             //выполнение cd
    char *s;
    if (argc == 1) {
        s = getenv("HOME");
        if (s == NULL) {
            return 1;
        } else chdir(s);
    } else if (argc > 2) {
        fprintf(stderr, "cd command accepts only 1 argument\n");
        return 1;
    } else if (chdir(argv[1]) != 0) {
        perror("cd error");
        return 1;
    }
    return 0;
}

void redirection(char **argv, int argc, short redir_in, short redir_out) {
    int index, f;

    if(redir_out == 1 && redir_in == 0) {
        index = find_sym(argv, argc, "<");                                      
        if(index == 0 || index == argc - 1) {
            printf("Wrong syntax.\n");
            exit(0);
        }
        f = open(argv[index + 1], O_RDONLY);
        if(f == -1) {
            fprintf(stderr, "wrong reference to file.\n");
            exit(0);
        }
        dup2(f, 0);
        close(f);
    }
    if(redir_in != 0 && redir_out == 0) {
        if(redir_in == 1) {
            index = find_sym(argv, argc, ">");                                    
            f = open(argv[index + 1], O_WRONLY | O_TRUNC | O_CREAT, 0664);
        } else {
            index = find_sym(argv, argc, ">>");                                      
            f = open(argv[index + 1], O_WRONLY | O_APPEND | O_CREAT, 0664);
        }
        if(index == 0 || index == argc - 1) {
            printf("Wrong syntax.\n");
            exit(0);
        }
        if(f == -1) {
            fprintf(stderr, "wrong redirection input to file.\n");
            exit(0);
        }
        dup2(f, 1);
        close(f);
    }

    if(redir_out != 0 && redir_in != 0) {
        int f_help, i_i;
        index = find_sym(argv, argc, "<");                                      //ДУБЛИРОВАНИЕ !!
        f = open(argv[index + 1], O_RDONLY);
        if(f == -1) {
            fprintf(stderr, "wrong reference to file.\n");
            exit(0);
        }
        i_i = index;
        if(redir_in == 1) {
            index = find_sym(argv, argc, ">");                                      
            f_help = open(argv[index + 1], O_WRONLY | O_TRUNC | O_CREAT, 0664);
        } else {
            index = find_sym(argv, argc, ">>");                                      
            f_help = open(argv[index + 1], O_WRONLY | O_APPEND | O_CREAT, 0664);
        }
        if(f_help == -1) {
            fprintf(stderr, "wrong redirection output to file.\n");
            exit(0);
        }
        dup2(f, 0);
        dup2(f_help, 1);
        close(f);
        close(f_help);
        argv[i_i] = NULL;
    }
    
    argv[index] = NULL;
}

/*void gaps(char **argv, int argc, int gap) {
    int gap_arr[gap];
    int gap_index[gap];
    char **sub;
    gap = 0;
    printf("*");
    for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "(") == 0) {
            gap_arr[gap] = 1;
            gap_index[gap] = find_sym(argv, argc, "(");
            strcpy(argv[gap_index[gap]], "0");
            gap++;
        } 
        if(strcmp(argv[i], ")") == 0) {
            gap_arr[gap] = -1;
            gap_index[gap] = find_sym(argv, argc, ")");
            strcpy(argv[gap_index[gap]], "0");
            gap++;
        }
    }

    for(int i = 0; i < gap; i++) {
        int k = 0; int j = i + 1;
        if(gap_arr[i] == 1) {
            k += gap_arr[i];
            while(k != 0) {
                k += gap_arr[j];
                j++;
            }
        }
        sub = sub_create(argv, gap_index[i], gap_index[j]);
        printf("*");
        status_analysis(sub, gap_index[j] - gap_index[i] - 1);
        exit(0);
    }
}*/

void cmd_exec(char **argv, int argc, pid_t pgid, short is_back) {
    short in = 0, out = 0, gap = count_sym(argv, argc, ")"); int status = 0;
    char **sub;
    if(find_sym(argv, argc, "<") != -1) out = 1;     
    if(find_sym(argv, argc, ">") != -1) in = 1;
    if(find_sym(argv, argc, ">>") != -1) in = 2;

    if(in != 0 || out != 0) redirection(argv, argc, in, out);
    if(is_back) {
        setpgid(0, pgid);                       
        int f = open("/dev/null", O_RDONLY);   
        dup2(f, 0);
        close(f);
        if(strcmp(argv[argc - 1], "&") == 0) argv[argc - 1] = NULL;
    }

    if(gap != 0) {
        //gaps(argv, argc, gap * 2);   
        exit(0);
    }                                                  //скобки ??
        
    if(execvp(argv[0], argv) == -1) {
        perror("error");
        exit(1);
    }
    exit(0);
}

int pipeline(char **argv, int argc) {
    int pipes = count_sym(argv, argc, "|");

    int cmd_n = pipes + 1, index1 = -1, index2 = 0, cnt = 0, status = 0;
    int fd_help, fd[2]; short is_forked = 0, is_back = 0;
    pid_t pid, pgid;

    pid_t *z_arr = malloc(cmd_n * sizeof(pid_t));

    char **sub_arr;

    if(strcmp(argv[0], "") == 0) {
        return 0;
    }
    if(strcmp(argv[0], "cd") == 0) return cd(argv, argc);
    if(strcmp(argv[argc - 1], "&") == 0) {                     //фоновый процесс?
        is_back = 1; 
    }
    if(is_back == 1 && argc == 1) {
        printf("No background commands.\n");
        return 0;
    }

    if(is_back) signal(SIGINT, SIG_IGN);
    else signal(SIGINT, SIG_DFL);

    if(is_back) {
        if (getpgrp() != getpid()) {                                                        //создаем фоновую группу 
            setpgid(0, 0); 
            is_forked = 1;
        } else if ((pid = fork()) == 0) pipeline(argv, argc);
        else return 0;
    }
    pgid = getpgrp();                                 

    if(pipes == 0) {
        if((pid = fork()) == 0) {                       //если одна команда
            cmd_exec(argv, argc, pgid, is_back);
        } else if(is_back) {
            z_arr[cnt] = pid;
            cnt++;
        } else if(pid > 0) wait(&status);
    } else {
        for(int i = 1; i <= cmd_n; i++) {                                  
            if(pipe(fd) == -1) return 1;
            if(i != cmd_n) {
                index2 = find_sym(argv, argc, "|");
                strcpy(argv[index2], "0");
            } else index2 = argc;
            sub_arr = sub_create(argv, index1, index2);

            if(i == 1) {                                    
                if((pid = fork()) == 0) {
                    dup2(fd[1], 1);
                    close(fd[0]);
                    close(fd[1]);
                    cmd_exec(sub_arr, index2 - index1 - 1, pgid, is_back);
                } else if(is_back) {
                    z_arr[cnt] = pid;
                    cnt++;
                } else wait(&status);
                index1 = index2;
                fd_help = fd[0];
                close(fd[1]);
            } else if(i == cmd_n) {                                            //последняя команда конвейера
                if((pid = fork()) == 0) {
                    dup2(fd_help, 0);
                    close(fd_help);
                    cmd_exec(sub_arr, index2 - index1 - 1, pgid, is_back);
                } 
                else if(is_back) {
                    z_arr[cnt] = pid;
                    cnt++;
                } else wait(&status);
                close(fd_help);
            } else {
                if((pid = fork()) == 0) {
                    dup2(fd_help, 0);
                    dup2(fd[1], 1);
                    close(fd[1]);
                    close(fd_help);
                    close(fd[0]);
                    cmd_exec(sub_arr, index2 - index1 - 1, pgid, is_back);
                } 
                else if(is_back) {
                    z_arr[cnt] = pid;
                    cnt++;
                } else wait(&status);
                index1 = index2;
                close(fd_help);
                close(fd[1]);
                fd_help = fd[0];
            }  
        }
    }
    
    if(is_back) {
        for(int i = 0; i < cnt; i++) {
            waitpid(z_arr[i], &status, 0);
            printf("\nDone: [%d]\n> ", z_arr[i]);
        }
    }
    free(z_arr);

    if(!is_back) {
        waitpid(pid, &status, 0);
    }
    if(pipes != 0) free(sub_arr);
    if(is_forked) exit(status);
    return status;
}

int status_analysis(char **argv, int argc) {
    int index1 = -1, index2 = 0, status = 0;

    char **sub;
    int count = 0;
    count += count_sym(argv, argc, "&&");
    count += count_sym(argv, argc, "||");

    if(count == 0) {
        return pipeline(argv, argc);
    }

    int or_and_arr[count];
    count = 0;
    for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "&&") == 0) {                                    //AND == 0
            or_and_arr[count] = 0;
            count++;

        } 
        if(strcmp(argv[i], "||") == 0) {                                   //OR == 1
            or_and_arr[count] = 1;
            count++;
        }
    }

    int is_first_cmd = 1;
    if(or_and_arr[0] == 0) {
        index2 = find_sym(argv, argc, "&&");
        strcpy(argv[index2], "0");
        sub = sub_create(argv, index1, index2);
        status = pipeline(sub, index2 - index1 - 1);
    } else {
        index2 = find_sym(argv, argc, "||");
        strcpy(argv[index2], "0");
        sub = sub_create(argv, index1, index2);
        status = pipeline(sub, index2 - index1 - 1);
    }
    index1 = index2;
    is_first_cmd = 0;
    for(int i = 1; i <= count; i++) {
        int j = 0;
        if(is_first_cmd) j = i;
        else j = i - 1;
        if(i == count) index2 = argc;
        if(or_and_arr[j] == 0)  { 
            if(index2 != argc) {
                index2 = find_sym(argv, argc, "&&");
                if(index2 != -1) strcpy(argv[index2], "0");
                else index2 = find_sym(argv, argc, "||");
            }
            sub = sub_create(argv, index1, index2);                                            //AND segment
            if(status == 0) {          
                status = pipeline(sub, index2 - index1 - 1);
            }
            !is_first_cmd;
        } else {
            if(index2 != argc) {
                index2 = find_sym(argv, argc, "||");
                if(index2 != -1) strcpy(argv[index2], "0");
                else index2 = find_sym(argv, argc, "&&");
            }
            sub = sub_create(argv, index1, index2);
            if(status != 0) {
                status = pipeline(sub, index2 - index1 - 1);
            }
            !is_first_cmd;
        }
        index1 = index2;
    }
    
    if(count != 0) free(sub);
    return status;
}
