#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void)
{
    char buffer[1024];
    int n;
    while (1)
    {
        n = read(0, buffer, sizeof(buffer));
        if (n == 0)
        {
            break;
        }

        buffer[n] = '\0';
        printf("echo: ");
        printf(buffer);
    }

    exit(0);
}