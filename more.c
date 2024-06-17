#include <stdio.h>    // 標準入出力関連の関数を使用するためのヘッダ
#include <stdlib.h>   // 汎用ユーティリティ関数（exitなど）を使用するためのヘッダ
#include <unistd.h>   // UNIX標準API（fork, pipe, read, writeなど）を使用するためのヘッダ
#include <fcntl.h>    // ファイル制御（open関数など）を使用するためのヘッダ
#include <sys/wait.h> // プロセス制御（wait関数）を使用するためのヘッダ

int main(int ac, char *av[])
{
    int fds[2];    // パイプ用のファイルディスクリプタ配列
    int infd;      // 入力ファイルのファイルディスクリプタ
    int cc;        // 読み込んだバイト数を保存する変数
    char buf[512]; // ファイルから読み込むデータを一時的に保存するバッファ
    pid_t pid;     // プロセスIDを保存する変数
    int status;    // 子プロセスの終了ステータスを保存する変数

    // 引数の数が正しくない場合、エラーメッセージを表示して終了
    if (ac != 2)
    {
        fprintf(stderr, "usage: %s file\n", av[0]);
        exit(1);
    }

    // 指定されたファイルを読み取り専用でオープン
    if ((infd = open(av[1], O_RDONLY)) < 0)
    {
        fprintf(stderr, "Cannot open: %s\n", av[1]);
        exit(1);
    }

    // パイプを作成
    if (pipe(fds) < 0)
    {
        perror("pipe");
        exit(1);
    }

    // 子プロセスを作成
    if ((pid = fork()) == 0)
    {
        // 子プロセスの処理
        close(0); // 標準入力を閉じる
        if (dup(fds[0]) < 0)
        { // パイプの読み込み端を標準入力に複製
            perror("dup");
            exit(1);
        }
        execl("/usr/bin/more", "more", (char *)0); // moreコマンドを実行
        exit(0);                                   // moreの実行が終了したら子プロセスを終了
    }

    // 親プロセスの処理
    while ((cc = read(infd, buf, sizeof(buf))) != 0)
    {                           // ファイルからデータを読み込む
        write(fds[1], buf, cc); // パイプの書き込み端にデータを書き込む
    }
    close(infd);   // 入力ファイルを閉じる
    close(fds[1]); // パイプの書き込み端を閉じる
    wait(&status); // 子プロセスの終了を待つ
}
