#include "kernel/types.h"
#include "user/user.h"

#define MAX_ENV_VALUE 256

int
main(int argc, char *argv[])
{
  

  // Set an environment variable
  if (setenv("TESTVAR", "Hello, xv6!") < 0) {
    fprintf(2, "setenv failed\n");
    exit(1);
  }


  char* value;
  // Get and print the environment variable
  if ((value = getenv("TESTVAR")) == 0) {
    fprintf(2, "getenv failed\n");
    exit(1);
  }

  printf("TESTVAR=%s\n", value);


  value = getenv("NONEXISTENT");

  // Try to get a non-existent variable
  if (getenv("NONEXISTENT") != 0) {
    fprintf(2, "getenv should have failed for non-existent variable\n");
    exit(1);
  }


   int pid = fork();
  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // Child process
    if ((value = getenv("TESTVAR")) == 0) {
      fprintf(2, "Child: getenv failed\n");
      exit(1);
    }
    printf("Child: TESTVAR=%s\n", value);

    // Modify the environment variable in the child
    if (setenv("TESTVAR", "Modified in child") < 0) {
      fprintf(2, "Child: setenv failed\n");
      exit(1);
    }

    if ((value = getenv("TESTVAR")) == 0) {
      fprintf(2, "Child: getenv failed after modification\n");
      exit(1);
    }
    printf("Child: Modified TESTVAR=%s\n", value);

    exit(0);
  } else {
    // Parent process
    wait(0);

    // Check if the parent's environment variable was not affected by the child's modification
    if ((value = getenv("TESTVAR")) == 0) {
      fprintf(2, "Parent: getenv failed after child modification\n");
      exit(1);
    }
    printf("Parent: TESTVAR after child modification=%s\n", value);
  }

  printf("Test completed successfully\n");
  exit(0);
}