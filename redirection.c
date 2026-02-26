#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

int checkForValidRedirection(int argCount, char *argList[]) {
    int redirIndex = -1;
    int redirCount = 0;

    for (int i = 0; i < argCount; i++) {
        if (strcmp(argList[i], ">") == 0) {
            redirCount++;

            //one > allowed
            if (redirCount > 1) {
                fprintf(stderr, "An error has occurred\n");
                return -2;
            }

            //not first token
            if (i == 0) {
                fprintf(stderr, "An error has occurred\n");
                return -2;
            }

            //only one file after >
            if (i + 1 >= argCount) {
                fprintf(stderr, "An error has occurred\n");
                return -2;
            }

            if (i + 2 < argCount) {
                fprintf(stderr, "An error has occurred\n");
                return -2;
            }

            redirIndex = i;
        }
    }

    return redirIndex; //-1 means no redirection
}

char* preprocess_redirection(const char* line) {
    int len = strlen(line);
    char* new_line = malloc(len * 3 + 1);
    if (!new_line) return NULL;

    int j = 0;
    for (int i = 0; i < len; i++) {
        if (line[i] == '>') {
            //add space before
            if (i > 0 && !isspace(line[i - 1])) {
                new_line[j++] = ' ';
            }
            //add '>'
            new_line[j++] = '>';
            //add space after
            if (i + 1 < len && !isspace(line[i + 1])) {
                new_line[j++] = ' ';
            }
        } else {
            new_line[j++] = line[i];
        }
    }
    new_line[j] = '\0';
    return new_line;
}
