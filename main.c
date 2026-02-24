#include "wish.h"
#include <stdio.h>
int main(int argc, char *argv[]) {
  char line[1024];
  FILE *a = performFileLogic(argc, argv);
  if(a == NULL) {
    return 1;
  }
  return wish(argc, argv, line, a);

  return 0;
}
