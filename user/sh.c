// Shell.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"

// Parsed command representation
#define EXEC 1
#define REDIR 2
#define PIPE 3
#define LIST 4
#define BACK 5

#define MAXARGS 10

#define SHELLRC "usr/.shellrc"

struct cmd
{
  int type;
};

struct execcmd
{
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd
{
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd
{
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd
{
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd
{
  int type;
  struct cmd *cmd;
};

void export(char *);
int fork1(void); // Fork but panics on failure.
void panic(char *);
void interpret(char *);
char *resolvepath(char *);
struct cmd *parsecmd(char *);
void runcmd(struct cmd *) __attribute__((noreturn));

void loadconfig()
{
  int fd;

  if ((fd = open(SHELLRC, 0)) < 0)
  {
    printf("sh: cannot read %s. but continue\n", "usr/.shellrc"); 
    return;
  }

  // copy from grep.c
  char buf[1024];
  int n, m;
  char *p, *q;

  m = 0;
  while ((n = read(fd, buf + m, sizeof(buf) - m - 1)) > 0)
  {
    m += n;
    buf[m] = '\0';
    p = buf;
    while ((q = strchr(p, '\n')) != 0)
    {
      *q = 0;
      interpret(p);
      p = q + 1;
    }
    if (m > 0)
    {
      m -= p - buf;
      memmove(buf, p, m);
    }
  }

  if (m > 0)
  {
    buf[m] = '\0';
    interpret(buf);
  }

  close(fd);
}

void export(char *env)
{
  char *eq = strchr(env, '=');
  if (eq == 0)
  {
    fprintf(2, "invalid export command\n");
    return;
  }

  char key[1024], value[1024];

  strncpy(key, env, eq - env);            // eq - env is the length of key
  strncpy(value, eq + 1, strlen(eq + 1)); // eq + 1 is the start of value, strlen(eq + 1) is the length of value
  setenv(key, value);
}

// Execute cmd.  Never returns.
void runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if (cmd == 0)
    exit(1);

  switch (cmd->type)
  {
  default:
    panic("runcmd");

  case EXEC:
    ecmd = (struct execcmd *)cmd;
    if (ecmd->argv[0] == 0)
      exit(1);
    for (int i = 0; ecmd->argv[i]; i++)
    {
      if (strstartwith(ecmd->argv[i], "$"))
      {
        char *value;
        value = getenv(ecmd->argv[i] + 1);
        ecmd->argv[i] = value;
      }
    }

    char *fullpath = resolvepath(ecmd->argv[0]);
    exec(fullpath, ecmd->argv);
    fprintf(2, "exec %s failed\n", ecmd->argv[0]);
    break;

  case REDIR:
    rcmd = (struct redircmd *)cmd;
    close(rcmd->fd);
    if (open(rcmd->file, rcmd->mode) < 0)
    {
      fprintf(2, "open %s failed\n", rcmd->file);
      exit(1);
    }
    runcmd(rcmd->cmd);
    break;

  case LIST:
    lcmd = (struct listcmd *)cmd;
    if (fork1() == 0)
      runcmd(lcmd->left);
    wait(0);
    runcmd(lcmd->right);
    break;

  case PIPE:
    pcmd = (struct pipecmd *)cmd;
    if (pipe(p) < 0)
      panic("pipe");
    if (fork1() == 0)
    {
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if (fork1() == 0)
    {
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait(0);
    wait(0);
    break;

  case BACK:
    bcmd = (struct backcmd *)cmd;
    if (fork1() == 0)
      runcmd(bcmd->cmd);
    break;
  }
  exit(0);
}

int getcmd(char *buf, int nbuf)
{
  write(2, "$ ", 2);
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if (buf[0] == 0) // EOF
    return -1;
  return 0;
}

void interpret(char *cmd)
{
  if (strstartwith(cmd, "cd "))
  {
    // Chdir must be called by the parent, not the child.
    if (chdir(cmd + 3) < 0)
      fprintf(2, "cannot cd %s\n", cmd + 3);
    return;
  }
  else if (strstartwith(cmd, "export "))
  {
    export(cmd + 7);
    return;
  }
  if (fork1() == 0)
    runcmd(parsecmd(cmd));
  wait(0);
}

int main(void)
{
  static char buf[100];
  int fd;

  // Ensure that three file descriptors are open.
  while ((fd = open("console", O_RDWR)) >= 0)
  {
    if (fd >= 3)
    {
      close(fd);
      break;
    }
  }

  loadconfig();

  // Read and run input commands.
  while (getcmd(buf, sizeof(buf)) >= 0)
  {
    buf[strlen(buf) - 1] = 0;
    interpret(buf);
  }

  exit(0);
}

void panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}

int fork1(void)
{
  int pid;

  pid = fork();
  if (pid == -1)
    panic("fork");
  return pid;
}

// PAGEBREAK!
//  Constructors

struct cmd *
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd *)cmd;
}

struct cmd *
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd *)cmd;
}

struct cmd *
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd *)cmd;
}

struct cmd *
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd *)cmd;
}

struct cmd *
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd *)cmd;
}
// PAGEBREAK!
//  Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while (s < es && strchr(whitespace, *s))
    s++;
  if (q)
    *q = s;
  ret = *s;
  switch (*s)
  {
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if (*s == '>')
    {
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if (eq)
    *eq = s;

  while (s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while (s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char **, char *);
struct cmd *parsepipe(char **, char *);
struct cmd *parseexec(char **, char *);
struct cmd *nulterminate(struct cmd *);

struct cmd *
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if (s != es)
  {
    fprintf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd *
parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);
  while (peek(ps, es, "&"))
  {
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd);
  }
  if (peek(ps, es, ";"))
  {
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

struct cmd *
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if (peek(ps, es, "|"))
  {
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd *
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while (peek(ps, es, "<>"))
  {
    tok = gettoken(ps, es, 0, 0);
    if (gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch (tok)
    {
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREATE | O_TRUNC, 1);
      break;
    case '+': // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd *
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if (!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if (!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd *
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if (peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd *)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while (!peek(ps, es, "|)&;"))
  {
    if ((tok = gettoken(ps, es, &q, &eq)) == 0)
      break;
    if (tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if (argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}
// NUL-terminate all the counted strings.
struct cmd *
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if (cmd == 0)
    return 0;

  switch (cmd->type)
  {
  case EXEC:
    ecmd = (struct execcmd *)cmd;
    for (i = 0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd *)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd *)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd *)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd *)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}


char *checkexisted(char *path)
{
  struct stat st;
  if (stat(path, &st) < 0)
  {
    return 0;
  }
  return path;
}

char *findpath(char *path, char *cmd)
{
  int fd;
  struct dirent de;
  char buf[512], *p;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "sh: cannot open to resolve %s\n", path);
    exit(1);
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
        continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if (strendwith(buf, cmd))
    {
      close(fd);
      char *fullpath = malloc(strlen(buf) + 1);
      strcpy(fullpath, buf);
      return fullpath;
    }
  }

  close(fd);
  return 0;
}

char *resolvepath(char *cmd)
{
  char* path = getenv("PATH");

  if (path == 0)
  {
    return cmd;
  }

  if(checkexisted(cmd) != 0)
  {
    return cmd;
  }

  char *p = path, *q;
  while ((q = strchr(p, ':')) != 0){
    *q = 0;
    char *fullpath = findpath(p, cmd);
    if (fullpath != 0)
    {
      return fullpath;
    }
    p = q + 1;
  }

  char *fullpath = findpath(p, cmd);
  if (fullpath != 0)
  {
    return fullpath;
  }

  return cmd;
}