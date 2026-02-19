#include <stdio.h>
#include <string.h>
int wish(int argc, char *argv[])
{
  char line[1024];
  int commandCount = 1;
  FILE *a = stdin;
  if (argc == 2) {
    a = fopen(argv[1], "r");
    if (a == NULL) {
        printf("Error opening file\n");
        return 1;
    }
  }
  
  while(1)
  {
    if(a == stdin) {
      printf("wish> ");
    }
    if(fgets(line, sizeof(line), a) == NULL) {
      break;
    }
    else {
      printf("line %d: %s", commandCount, line);
      commandCount++;
    }
  }
}
