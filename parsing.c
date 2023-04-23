#include "shelllib.h"

char *keyboard_enter(void) {                        //ввод строк с клавиатуры
    int unsigned str_length, k = 0;
    char *s = malloc(ADD_M);
    if (!s) return NULL;

    fflush(stdout);
    printf("> ");
    while(fgets(s + k, ADD_M, stdin)) {
        str_length = strlen(s);
        if(s[str_length - 1] != '\n') {
            k = k + ADD_M - 1;
            s = realloc(s, k + ADD_M);

            if(!s) return NULL;
        } else {
            s[str_length - 1] = '\0';
            return s;
        }
    }
    return NULL;
}

char *file_enter(FILE *f) {                        //ввод из файла
   int unsigned str_length = 0, k = 0;

    char *s = malloc(ADD_M);
    if (!s) return NULL;

    while(fgets(s + k, ADD_M, f)) {
        str_length = strlen(s);
        if(s[str_length - 1] != '\n' && !feof(f)) {
            k = k + ADD_M - 1;
            s = realloc(s, k + ADD_M);

            if(!s) return NULL;
        } else {
            if(s[str_length - 1] == '\n') s[str_length - 1] = '\0';
            return s;
        }
    }
    return NULL;
}

void mem_all(char *word, int l_c, char **w_a, int *w_c) {       //выделение памяти на слова в массиве
    word[l_c] = '\0';
    w_a[*w_c] = malloc(l_c + 1);
    strcpy(w_a[*w_c], word);
    (*w_c)++;
}

char **parse(char **w_arr, char *s, int *counter, int *current_args, short *pp, short *dd) {                 //разбивка строк на слова
    short is_closing_quote_mark = 0, is_first_space = 0, is_prev_special = 0, is_pipe = 0, is_div = 0, gaps_check = 0;         
    int let_counter = 0, word_counter = *counter, word_mem = ((*counter) / 10 + 1) * ADD_M, let_mem = ADD_M;
    char *word = malloc(ADD_M);

    for(int i = 0; i < strlen(s); i++) {
        if(s[i] != ' ') {                                           //ниже: обработка спецсимволов
            if(s[i] == '&' || s[i] == '|' || s[i] == ';' || s[i] == '>' || s[i] == '<' || s[i] == '(' || s[i] == ')') {
                if(s[i] == '|') is_pipe++;  
                if(s[i] == ';') is_div++;                        //сделаем счетчик команд конвейера

                if(s[i] == '(') gaps_check++;
                if(s[i] == ')') gaps_check--; 
                if(gaps_check < 0) {
                    printf("Wrong gaps sequence");
                    exit(0);
                }

                if(i != 0 && is_first_space != 1 && is_prev_special != 1) {                   //если спецсимвол первый в строке или стоит после пос-ти пробелов
                    if(word_counter == word_mem) {              //то нет предшествующего незаписанного слова
                        word_mem += ADD_M;
                        w_arr = realloc(w_arr, word_mem * sizeof(char*));
                    }
                    mem_all(word, let_counter, w_arr, &word_counter);
                    (*current_args)++;
                }

                let_counter = 0;
                word[let_counter] = s[i];
                if(((i + 1) != strlen(s)) && (s[i + 1] == '&' || s[i + 1] == '|' || s[i + 1] == '>')) {  //проверка для парных символов
                    let_counter++;
                    i++;
                    word[let_counter] = s[i];

                    if(s[i] == '|') is_pipe--;                          //исключим из счетчика вариант "||" !!
                }
                let_counter++;
                mem_all(word, let_counter, w_arr, &word_counter);
                (*current_args)++;
                let_counter = 0;
                is_prev_special = 1;
                continue;
            }
            is_first_space = 0;
            is_prev_special = 0;
            if(let_counter == let_mem - 1) {
                let_mem += ADD_M;
                word = realloc(word, let_mem);
            }
            if(s[i] == '"' && is_closing_quote_mark == 0) {        //обработка кавычек
                is_closing_quote_mark = 1;
                continue;
            } 
            if(s[i] == '"' && is_closing_quote_mark == 1) {
                is_closing_quote_mark = 0;
                continue;
            }
            word[let_counter] = s[i];
            let_counter++;
        } else {
            if(is_closing_quote_mark == 1) {               //обработка пробелов внутри кавычек
                word[let_counter] = ' ';
                let_counter++;
                continue;
            }
            if(is_first_space != 0) continue;
            is_first_space = 1;

            if(let_counter != 0) {
                if(word_counter == word_mem) {
                    word_mem += ADD_M;
                    w_arr = realloc(w_arr, word_mem * sizeof(char*));
                }
                mem_all(word, let_counter, w_arr, &word_counter);
                (*current_args)++;
            }
            let_counter = 0;
        }
    }

    if(gaps_check != 0) {
        exit(0);
    }

    if(is_first_space == 0 && is_prev_special == 0) {                                 //запись последнего слова, если не пробел и не спецсимвол
        if(word_counter == word_mem) {
            word_mem += ADD_M;
            w_arr = realloc(w_arr, word_mem * sizeof(char*));
        }
        mem_all(word, let_counter, w_arr, &word_counter);
        (*current_args)++;
    }

    free(word);
    *counter = word_counter;
    *pp = is_pipe;
    *dd = is_div;

    return w_arr;
}

void output(char **arr, int size) {                         //вывод массива слов
    printf("-----------\n");
    if(size == 0) printf("No words entered.\n");
    for(int i = 0; i < size; i++) {
        printf("%s\n", arr[i]);
    }
}

void parse_exec(char **words_arr, char *s, int *count) {                            //создаем подмассив из текущей строки
    int cur_count = 0, c_count = *count, index1 = -1, index2 = 0; short pipe_flag = 0, div_flag = 0;
    words_arr = parse(words_arr, s, &c_count, &cur_count, &pipe_flag, &div_flag);
    char *cur_arr[cur_count + 1];
    char **curr_sub;
    for(int i = 0; i < cur_count; i++) {
        cur_arr[i] = words_arr[c_count - cur_count + i];
    }
    cur_arr[cur_count] = NULL;

    if(strcmp(cur_arr[cur_count - 1], ";") == 0) {
        cur_arr[cur_count - 1] = NULL;
        cur_count--;
        div_flag--;
    }
    if(div_flag != 0) {
        for(int i = 0; i <= div_flag; i++) {
            if(i == div_flag) index2 = cur_count;
            else index2 = find_sym(cur_arr, cur_count, ";");
            if(i != div_flag) strcpy(cur_arr[index2], "0");
            curr_sub = sub_create(cur_arr, index1, index2);
            status_analysis(curr_sub, index2 - index1 - 1);
            index1 = index2;
        }
    } else {
        status_analysis(cur_arr, cur_count);
    }
    *count = c_count;
    free(curr_sub);
}

int find_sym(char ** arr, int n, char *c) {
    for(int i = 0; i < n; i++) {
        if(strcmp(arr[i], c) == 0) {
            return i;
        }
    }
    return -1;
}

int count_sym(char **arr, int n, char *c) {
    int cnt = 0;
    for(int i = 0; i < n; i++) {
        if(strcmp(arr[i], c) == 0) {
            cnt++;
        }
    }
    return cnt;
}

char **sub_create(char **arr, int i1, int i2) {
    char **sub = malloc((i2 - i1) * sizeof(char*));
    for(int i = i1 + 1, j = 0; i < i2; i++, j++) {
        sub[j] = malloc(strlen(arr[i]) + 1);
        strcpy(sub[j], arr[i]);
    }
    sub[i2] = NULL;
    return sub;
}
