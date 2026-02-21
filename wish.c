#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 10

char *pathList[MAX_COMMAND_LENGTH] = {"/bin", NULL}; 
int pathCount = 1;

int handleCommands(int argc, char *argv[]);

int wish(int argc, char *argv[])
{
  //BATCH MODE
  char line[1024];
  bool batchMode = false;
  int commandCount = 1;
  FILE *a = stdin;
  if (argc == 2) {
    a = fopen(argv[1], "r");
    if (a == NULL) {
        fprintf(stderr, "An error has occurred\n");
        return 1;
    }
    else{
      batchMode = true;
    }

  }
  
  //INTERACTIVE MODE
  while (1) {
    if (a == stdin) {
        printf("wish> ");
    }

    if (fgets(line, sizeof(line), a) == NULL) {
        break;
    }
    else if(batchMode == true){
      printf("%s",line);
      fflush(stdout);
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

    if (argCount == 0) continue;

    // built‑ins
    int isExternal = handleCommands(argCount, argList);

    // external commands
    if (isExternal) {
        pid_t pid = fork();

      if (pid == 0) {
      // CHILD: try each directory in PATH
        for (int i = 0; i < pathCount; i++) {
          char fullpath[256];
          snprintf(fullpath, sizeof(fullpath), "%s/%s", pathList[i], argList[0]);
          execv(fullpath, argList);
        }

      // If we reach here, all execv calls failed
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
int handleCommands(int argc, char *argv[]) {
    // exit
    if (strcmp(argv[0], "exit") == 0) {
        if (argc != 1) {
            fprintf(stderr, "An error has occurred\n");
            return 0;
        }
        exit(0);
    }

    // cd
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

    // not a built‑in → external command
    return 1;
}

