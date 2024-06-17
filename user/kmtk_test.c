#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int kmtk_result = myfds();
    printf("kmtk() returned %d\n", kmtk_result);

    int fds[2];
    pipe(fds);

    int kmtk_result2 = myfds();
    printf("kmtk() returned %d\n", kmtk_result2);

    close(fds[0]);
    close(fds[1]);
    close(0);
    close(2);

    int kmtk_result3 = myfds();
    printf("kmtk() returned %d\n", kmtk_result3);

    exit(0);
}
