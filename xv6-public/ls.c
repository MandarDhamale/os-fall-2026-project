#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char *
fmtname(char *path, int is_dir)
{
  static char buf[DIRSIZ + 2]; // Room for name, slash, and spaces
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  int len = strlen(p);
  if (len >= DIRSIZ)
    return p;

  memmove(buf, p, len);
  
  if (is_dir) {
      buf[len] = '/'; // Attach slash directly to the directory name
      memset(buf + len + 1, ' ', DIRSIZ - len); // Pad the rest with spaces
  } else {
      memset(buf + len, ' ', DIRSIZ - len + 1); // Pad with spaces (no slash)
  }
  
  buf[DIRSIZ + 1] = '\0';
  return buf;
}

void ls(char *path, int show_hidden)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, 0)) < 0)
  {
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0)
  {
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type)
  {
  case T_FILE:
    printf(1, "%s %d %d %d\n", fmtname(path, 0), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
    {
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
      if (de.inum == 0)
        continue;

      // Hide logic: if show_hidden is 0 and name starts with '.', skip
      if (!show_hidden && de.name[0] == '.')
      {
        continue;
      }

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if (stat(buf, &st) < 0)
      {
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }

      printf(1, "%s %d %d %d\n", fmtname(buf, st.type == T_DIR), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  int i;
  int show_hidden = 0;
  int start_idx = 1;

  // Check for -a flag
  if (argc > 1 && strcmp(argv[1], "-a") == 0)
  {
    show_hidden = 1;
    start_idx = 2; // Skip the -a argument
  }

  // If no directory argument provided, list current directory
  if (argc == start_idx)
  {
    ls(".", show_hidden);
    exit();
  }

  // Iterate over provided directories
  for (i = start_idx; i < argc; i++)
  {
    ls(argv[i], show_hidden);
  }

  exit();
}