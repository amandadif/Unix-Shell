#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 10
#define MAX_LINE 1024

//Global path for searching external commands
char *pathList[MAX_COMMAND_LENGTH] = {"/bin", NULL};
int pathCount = 1;

//Forward declaration because it is used in wish()
int handleBuiltInCommands(int argc, char *argv[]);


int wish(int argc, char *argv[], char line[], FILE *a)
{
  while (1) {
    if (a == stdin) {
        printf("wish> ");
    }
    fflush(stdout);

    if (fgets(line, MAX_LINE, a) == NULL) {
        break;
    }

    line[strcspn(line, "\r\n")] = '\0';

    // tokenize
    int argCount = 0;
    char *argList[MAX_COMMAND_LENGTH];
    char *token = strtok(line, " \t\n");
    while (token != NULL) {
        argList[argCount++] = token;
        token = strtok(NULL, " \t\n");
    }
    argList[argCount] = NULL;

    if (argCount == 0) {
      continue;
    }

    int notBuiltIn = handleBuiltInCommands(argCount, argList);

    //execute external commands
    if (notBuiltIn == 1) {
      if(pathCount == 0) {
        fprintf(stderr, "An error has occurred\n");
        continue;
      }
      pid_t pid = fork();

      if (pid == 0) {
        //child
        for (int i = 0; i < pathCount; i++) {
          char fullpath[256];
          snprintf(
            fullpath,
            sizeof(fullpath),
            "%s/%s",
            pathList[i],
            argList[0]
          );
          execv(fullpath, argList);
        }

        //execv failed
        fprintf(stderr, "An error has occurred\n");
        exit(1);

      }
      else if (pid > 0) {
        wait(NULL);
      }
      else {
        fprintf(stderr, "An error has occurred\n");
      }
    }
  }
  return 0;
}
int handleBuiltInCommands(int argc, char *argv[]) {
    //exit
    if (argc == 0) {
        return 0;
    }
    if (strcmp(argv[0], "exit") == 0) {
        if (argc != 1) {
            fprintf(stderr, "An error has occurred\n");
            return 0;
        }
        exit(0);
    }

    //cd
    if (strcmp(argv[0], "cd") == 0) {
        if (argc != 2) {
            fprintf(stderr, "An error has occurred\n");
            return 0;
        }
        if (chdir(argv[1]) != 0) {
           fprintf(stderr, "An error has occurred\n");
        }
        return 0;   // built‑in handled
    }

    //path
    if (strcmp(argv[0], "path") == 0) {

      //reset path to 0
      pathCount = 0;

      //new path
      for (int i = 1; i < argc; i++) {
        pathList[pathCount] = strdup(argv[i]);
        pathCount++;
      }

      return 0;
    }

    //No built in command found
    return 1;
}

FILE* performFileLogic(int argc, char *argv[]) {
  FILE *a = stdin;
  if (argc == 2) {
    a = fopen(argv[1], "r");
    if (a == NULL) {
        fprintf(stderr, "An error has occurred\n");
        return NULL;
    }

  }
  return a;
}
