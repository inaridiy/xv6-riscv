#include "kernel/types.h"
#include "user/user.h"

void sieve(int read_fd)
{
    int prime;
    if (read(read_fd, &prime, sizeof(prime)) <= 0)
    {
        close(read_fd);
        exit(0);
    }

    printf("prime %d\n", prime);

    int new_fd[2];
    pipe(new_fd);

    if (fork() == 0)
    {
        close(new_fd[1]); // 新しい子プロセスで書き込み端を閉じる
        sieve(new_fd[0]); // レシーブ側パイプで再帰的に呼び出し
    }
    else
    {
        close(new_fd[0]); // 親プロセスでは読み込み端を閉じる
        int num;
        while (read(read_fd, &num, sizeof(num)) > 0)
        {
            if (num % prime != 0)
            {
                write(new_fd[1], &num, sizeof(num)); // 割り切れない数を次のプロセスに渡す
            }
        }
        close(new_fd[1]);
        wait(0); // 子プロセスの終了を待つ
        close(read_fd);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int check_max = 1000000000;
    int fds[2];

    pipe(fds);

    if (fork() != 0)
    {
        close(fds[0]); // 親プロセスは読み込み側を閉じる
        for (int i = 2; i <= check_max; i++)
        {
            write(fds[1], &i, sizeof(i));
        }
        close(fds[1]); // 数値の書き込み完了後に書き込み側を閉じる
        wait(0);       // 子プロセスの終了を待つ
    }
    else
    {
        close(fds[1]); // 子プロセスは書き込み側を閉じる
        sieve(fds[0]); // 読み込み側で数値をフィルタリング
    }

    exit(0);
}
