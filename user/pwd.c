#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define BUF_SIZE 512

void strncpy(char *dst, const char *src, int n)
{
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dst[i] = src[i];
    }
    for (; i < n; i++)
    {
        dst[i] = '\0';
    }
}

void get_path(int fd, char *path)
{
    int fd_parent;
    struct dirent de;
    struct stat st, st_parent;

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "Error: Cannot stat directory.\n");
        return;
    }

    fd_parent = open("..", 0);
    if (fd_parent < 0)
    {
        fprintf(2, "Error: Cannot open parent directory.\n");
        return;
    }

    if (fstat(fd_parent, &st_parent) < 0)
    {
        fprintf(2, "Error: Cannot stat parent directory.\n");
        close(fd_parent);
        return;
    }

    while (read(fd_parent, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;
        if (de.inum == st.ino)
        {
            strncpy(path + strlen(path), "/", 1);
            strncpy(path + strlen(path), de.name, DIRSIZ);
            break;
        }
    }

    close(fd_parent);

    if (st.ino != st_parent.ino)
    {
        get_path(fd_parent, path);
    }
}

int main(int argc, char *argv[])
{
    char path[BUF_SIZE] = {0};
    int fd;

    fd = open(".", 0);
    if (fd < 0)
    {
        fprintf(2, "Error: Cannot open current directory.\n");
        exit(1);
    }

    get_path(fd, path);
    printf("%s\n", path);

    close(fd);
    exit(0);
}
