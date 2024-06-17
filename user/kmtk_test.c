#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int kmtk_result = kmtk();
    printf("kmtk() returned %d\n", kmtk_result);
    exit(0);
}
