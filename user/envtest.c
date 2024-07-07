#include "kernel/types.h"
#include "user/user.h"

#define MAX_ENV_VALUE 256

int
main(int argc, char *argv[])
{
  char value[MAX_ENV_VALUE];
  

  // Set an environment variable
  if (setenv("TESTVAR", "Hello, xv6!") < 0) {
    fprintf(2, "setenv failed\n");
    exit(1);
  }

  // Get and print the environment variable
  if (getenv("TESTVAR") == 0) {
    fprintf(2, "getenv failed\n");
    exit(1);
  }

  printf("TESTVAR=%s\n", value);

  // Try to get a non-existent variable
  if (getenv("NONEXISTENT") != 0) {
    fprintf(2, "getenv should have failed for non-existent variable\n");
    exit(1);
  }

  printf("Test completed successfully\n");
  exit(0);
}