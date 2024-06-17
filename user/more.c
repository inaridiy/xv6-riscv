#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int fds[2];
    int fd;
    int n;
    char buf[512];

    if (argc != 2)
    {
        printf(2, "usage: %s file\n", argv[0]);
        exit(0);
    }

    if ((fd = open(argv[1], 0)) < 0)
    { // O_RDONLYはxv6では通常0です
        printf(2, "Cannot open %s\n", argv[1]);
        exit(0);
    }

    if (pipe(fds) < 0)
    {
        printf(2, "pipe failed\n");
        exit(0);
    }

    if (fork() == 0)
    {
        close(0);
        dup(fds[0], 0);
        close(fds[0]);
        close(fds[1]);
        char *args[] = {"more", 0}; // xv6ではexecを使用して直接配列を渡す
        exec("more", args);
        printf(2, "exec more failed\n");
        exit(0);
    }

    close(fds[0]);
    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        write(fds[1], buf, n);
    }
    close(fd);
    close(fds[1]);
    wait(0);
    exit(0);
}
