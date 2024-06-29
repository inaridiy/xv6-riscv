#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 80

char lines[MAX_LINES][MAX_LINE_LENGTH];
int num_lines = 0;
int cursor_x = 0;
int cursor_y = 0;

void clear_screen()
{
    printf("\033[2J");
    printf("\033[H");
}

void move_cursor(int x, int y)
{
    printf("\033[%d;%dH", y + 1, x + 1);
}

void display_lines()
{
    clear_screen();
    for (int i = 0; i < num_lines; i++)
    {
        printf("%s\n", lines[i]);
    }
    move_cursor(cursor_x, cursor_y);
}

void insert_char(char c)
{
    if (cursor_x < MAX_LINE_LENGTH - 1)
    {
        memmove(&lines[cursor_y][cursor_x + 1], &lines[cursor_y][cursor_x], MAX_LINE_LENGTH - cursor_x - 1);
        lines[cursor_y][cursor_x] = c;
        cursor_x++;
    }
}

void delete_char()
{
    if (cursor_x > 0)
    {
        memmove(&lines[cursor_y][cursor_x - 1], &lines[cursor_y][cursor_x], MAX_LINE_LENGTH - cursor_x);
        cursor_x--;
    }
}

void new_line()
{
    if (num_lines < MAX_LINES - 1)
    {
        memmove(&lines[cursor_y + 1], &lines[cursor_y], (MAX_LINES - cursor_y - 1) * MAX_LINE_LENGTH);
        memset(lines[cursor_y + 1], 0, MAX_LINE_LENGTH);
        num_lines++;
        cursor_y++;
        cursor_x = 0;
    }
}

void load_file(char *filename)
{
    int fd = open(filename, 0);
    if (fd < 0)
    {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }

    char buffer[MAX_LINE_LENGTH];
    int n;
    num_lines = 0;
    while ((n = read(fd, buffer, MAX_LINE_LENGTH)) > 0)
    {
        int start = 0;
        for (int i = 0; i < n; i++)
        {
            if (buffer[i] == '\n' || i == n - 1)
            {
                int len = i - start;
                if (i == n - 1 && buffer[i] != '\n')
                    len++;
                memcpy(lines[num_lines], &buffer[start], len);
                lines[num_lines][len] = '\0';
                num_lines++;
                start = i + 1;
            }
        }
    }
    close(fd);
}

void save_file(char *filename)
{
    int fd = open(filename, 0);
    if (fd < 0)
    {
        printf("Error: Cannot open file %s for writing\n", filename);
        return;
    }

    for (int i = 0; i < num_lines; i++)
    {
        write(fd, lines[i], strlen(lines[i]));
        write(fd, "\n", 1);
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(2, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    load_file(argv[1]);
    display_lines();

    char c;
    while (read(0, &c, 1) > 0)
    {
        switch (c)
        {
        case 17: // Ctrl+Q
            save_file(argv[1]);
            exit(0);
        case 19: // Ctrl+S
            save_file(argv[1]);
            break;
        case '\r':
        case '\n':
            new_line();
            break;
        case 127: // Backspace
            delete_char();
            break;
        default:
            insert_char(c);
            break;
        }
        display_lines();
    }

    return 0;
}