#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void)
{
    int p[2];
    char *argv[2];
    argv[1] = 0;

    // パイプ作成
    pipe(p);

    if (fork() == 0)
    {
        // 子プロセス
        close(1);    // 標準出力を閉じる
        dup(p[1]);   // パイプの書き込み側を標準出力に複製
        close(p[0]); // パイプの読み込み側は不要なので閉じる
        close(p[1]);

        argv[0] = "ls";
        exec("ls", argv);
        exit(1);
    }
    else
    {
        // 親プロセス
        close(0);    // 標準入力を閉じる
        dup(p[0]);   // パイプの読み込み側を標準入力に複製
        close(p[1]); // パイプの書き込み側は不要なので閉じる
        close(p[0]);

        argv[0] = "wc";
        exec("wc", argv);
    }
    exit(0);
}
