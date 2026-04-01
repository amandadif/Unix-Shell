#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include "redirection.h"
#include "ampersand.h"

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 10
#define MAX_LINE 1024

//Global path for searching external commands
char *pathList[MAX_COMMAND_LENGTH] = {"/bin", NULL};
int pathCount = 1;

//Forward declaration because it is used in wish()
int handleBuiltInCommands(int argc, char *argv[]);
pid_t execute(char *argList[], int start, pid_t *pids, int pidCount, int i);

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

    char* temp = preprocess_redirection(line);
    strcpy(line, temp);
    free(temp);


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

      int validRedirection = checkForValidRedirection(argCount, argList);
      int amountOfAmpersand = checkForValidAmpersand(argCount, argList);
      if(validRedirection == -2 || amountOfAmpersand == -100) {
        continue;
      }


      int start = 0;
      pid_t pids[MAX_COMMAND_LENGTH];
      int pidCount = 0;

      for (int i = 0; i <= argCount; i++) {

        int isAmpersand = 0;

        //detect &
        if (i < argCount && argList[i] && strcmp(argList[i], "&") == 0) {
          isAmpersand = 1;
        }


        if (i == argCount || isAmpersand) {

          if (argList[start] == NULL) {
            start = i + 1;
            continue;
          }

          if (isAmpersand) {
            argList[i] = NULL;
          }
          pid_t pid = execute(argList, start, pids, pidCount, i);
          if (pid > 0) {
            if (!isAmpersand) {
              wait(NULL);
            }
            else {
              pids[pidCount++] = pid;
            }
          }
          else {
            fprintf(stderr, "An error has occurred\n");
          }

          start = i + 1;

        }
      }

      //wait for all background processes
      for (int i = 0; i < pidCount; i++) {
        wait(NULL);
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

  //batch
  if (argc == 2) {
    a = fopen(argv[1], "r");
    if (a == NULL) {
      char error_message[] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, 22);
      exit(1);  // MUST exit
    }
  }
  //too many arguments
  else if (argc > 2) {
    char error_message[] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, 22);
    exit(1);
  }

  return a;  //stdin if argc == 1, or first file if argc == 2
}

pid_t execute(char *argList[], int start, pid_t *pids, int pidCount, int i) {

  pid_t pid = fork();

  if (pid == 0) {
    //redirection
    for (int k = start; k < i; k++) {
      if (argList[k] && strcmp(argList[k], ">") == 0) {
        int fd = open(argList[k + 1],
            O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
          fprintf(stderr, "An error has occurred\n");
          exit(1);
        }

        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);

        argList[k] = NULL;
        break;
      }
    }
    for (int j = 0; j < pathCount; j++) {
      char fullpath[256];
      snprintf(fullpath, sizeof(fullpath),
          "%s/%s", pathList[j], argList[start]);
      execv(fullpath, &argList[start]);
    }

    fprintf(stderr, "An error has occurred\n");
    exit(1);
  }
  return pid;
}
