// Simple grep.  Only supports ^ . * $ operators.

#include "types.h"
#include "stat.h"
#include "user.h"

char buf[1024];
int flag_i = 0;
int flag_n = 0;
int flag_v = 0;

int match(char*, char*);

char
tolower_char(char c)
{
  if(c >= 'A' && c <= 'Z')
    return c + ('a' - 'A');
  return c;
}

int
char_eq(char a, char b)
{
  if(flag_i)
    return tolower_char(a) == tolower_char(b);
  return a == b;
}

void
grep(char *pattern, int fd)
{
  int n, m;
  char *p, *q;
  int line_num = 1;

  m = 0;
  while((n = read(fd, buf+m, sizeof(buf)-m-1)) > 0){
    m += n;
    buf[m] = '\0';
    p = buf;
    while((q = strchr(p, '\n')) != 0){
      *q = 0;
      int matched = match(pattern, p);
      if((matched && !flag_v) || (!matched && flag_v)){
        if(flag_n)
          printf(1, "%d:", line_num);
        *q = '\n';
        write(1, p, q+1 - p);
      }
      line_num++;
      p = q+1;
    }
    if(p == buf)
      m = 0;
    if(m > 0){
      m -= p - buf;
      memmove(buf, p, m);
    }
  }
  if(m > 0){
    buf[m] = '\0';
    int matched = match(pattern, buf);
    if((matched && !flag_v) || (!matched && flag_v)){
      if(flag_n)
        printf(1, "%d:", line_num);
      printf(1, "%s\n", buf);
    }
  }
}

int
main(int argc, char *argv[])
{
  int fd, i, j;
  char *pattern;
  int arg_idx = 1;

  while(arg_idx < argc && argv[arg_idx][0] == '-' && argv[arg_idx][1] != '\0'){
    for(j = 1; argv[arg_idx][j] != '\0'; j++){
      if(argv[arg_idx][j] == 'i')
        flag_i = 1;
      else if(argv[arg_idx][j] == 'n')
        flag_n = 1;
      else if(argv[arg_idx][j] == 'v')
        flag_v = 1;
      else {
        printf(2, "grep: invalid option -%c\n", argv[arg_idx][j]);
        exit();
      }
    }
    arg_idx++;
  }

  if(arg_idx >= argc){
    printf(2, "usage: grep [-inv] pattern [file ...]\n");
    exit();
  }
  pattern = argv[arg_idx++];

  if(arg_idx >= argc){
    grep(pattern, 0);
    exit();
  }

  for(i = arg_idx; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "grep: cannot open %s\n", argv[i]);
      exit();
    }
    grep(pattern, fd);
    close(fd);
  }
  exit();
}

// Regexp matcher from Kernighan & Pike,
// The Practice of Programming, Chapter 9.

int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || char_eq(re[0], *text)))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (char_eq(*text++, c) || c=='.'));
  return 0;
}
