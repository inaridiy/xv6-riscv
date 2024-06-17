#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char buf[100];

void simple_shell()
{
    int pid, wpid;
    while (1)
    {
        // プロンプト表示
        write(1, "$$$ ", 2);

        // コマンド入力
        memset(buf, 0, sizeof(buf));
        gets(buf, sizeof(buf));
        buf[strlen(buf) - 1] = 0; // 改行文字の削除

        if (buf[0] == 0)
            continue; // 入力がない場合はスキップ

        // exitコマンド
        if (strcmp(buf, "exit") == 0)
        {
            exit(0);
        }

        // コマンド実行
        if ((pid = fork()) == 0)
        {
            // 子プロセスでコマンド実行
            char *argv[2];
            argv[0] = buf; // コマンド
            argv[1] = 0;   // 引数の終わり
            exec(buf, argv);
            printf("exec %s failed\n", buf);
            exit(1);
        }
        else
        {
            // 親プロセスで子プロセスの終了を待つ
            while ((wpid = wait((int *)0)) >= 0 && wpid != pid)
                ; // 子プロセスの終了を待つ
        }
    }
}

int main(void)
{
    simple_shell();
    exit(0);
}
