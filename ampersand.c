#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

int checkForValidAmpersand(int argCount, char *argList[]) {
    int ampIndex = -1;
    int ampCount = 0;

    for (int i = 0; i < argCount; i++) {
        if (strcmp(argList[i], "&") == 0) {
            ampCount++;
            if (argCount == 1 && strcmp(argList[0], "&") == 0) {
              return -100;
            }

            ampIndex = i;
        }
    }

    return ampCount;
}
