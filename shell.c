#include "shelllib.h"

int main(int argc, char *argv[]) {
    char c[256]; int count = 0, is_right_sym = 0;
    FILE *f;
    char *s = malloc(ADD_M);
    char **words_arr = malloc(ADD_M * sizeof(char*));

    printf("----------- SHELL INTERPRETER -----------\n");
    printf("To enter data from a file press 'f'. To enter from the keyboard press 'k': ");
    do {                                                     //проверка правильности введеных символов
        scanf("%s", c);
        if(strcasecmp(c, "k") == 0 || strcasecmp(c, "f") == 0) is_right_sym = 1;
        if(is_right_sym == 0) printf("Wrong input. Try again (f/k): ");
    } while(is_right_sym == 0);
    getchar();

    if(strcasecmp(c, "k") == 0) {
        printf("To stop entering press Ctrl+D.\n");
        while(s != NULL) {
            s = keyboard_enter();
            if(s != NULL) parse_exec(words_arr, s, &count);
        }
        printf("stopped\n");
    }
    if(strcasecmp(c, "f") == 0) {
        printf("Inputing from file..\n");
        f = fopen(argv[1], "r");
        while(s != NULL) {
            s = file_enter(f);
            if(s != NULL) parse_exec(words_arr, s, &count);
        }
        fclose(f);
    }

    printf("\n-----------------------------------------\n");

    free(s);
    free(words_arr);
    return 0;
}
