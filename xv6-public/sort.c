#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_LINES 2048
#define MAX_LINE_LEN 512

char *lines[MAX_LINES];
int num_lines = 0;

int flag_r = 0;
int flag_n = 0;
int flag_u = 0;

// Helper to parse leading integer (supporting optional leading spaces and +/- sign)
int
parse_num(const char *s)
{
  int sign = 1;
  int val = 0;

  while (*s == ' ' || *s == '\t')
    s++;

  if (*s == '-') {
    sign = -1;
    s++;
  } else if (*s == '+') {
    s++;
  }

  while (*s >= '0' && *s <= '9') {
    val = val * 10 + (*s - '0');
    s++;
  }

  return sign * val;
}

// Compare two lines according to active flags (-n and -r)
int
compare_lines(const char *a, const char *b)
{
  int cmp;

  if (flag_n) {
    int na = parse_num(a);
    int nb = parse_num(b);
    if (na < nb)
      cmp = -1;
    else if (na > nb)
      cmp = 1;
    else
      cmp = strcmp(a, b);
  } else {
    cmp = strcmp(a, b);
  }

  if (flag_r)
    return -cmp;
  return cmp;
}

// Check if two lines are considered duplicates for -u
int
is_duplicate(const char *a, const char *b)
{
  if (flag_n) {
    return parse_num(a) == parse_num(b);
  }
  return strcmp(a, b) == 0;
}

// Read all lines from fd into lines[]
void
read_lines(int fd)
{
  static char buf[512];
  static char line_buf[MAX_LINE_LEN];
  int n, i;
  int line_len = 0;

  while ((n = read(fd, buf, sizeof(buf))) > 0) {
    for (i = 0; i < n; i++) {
      char c = buf[i];
      if (c == '\r')
        continue;
      if (c == '\n') {
        line_buf[line_len] = '\0';
        if (num_lines < MAX_LINES) {
          char *p = malloc(line_len + 1);
          if (p == 0) {
            printf(2, "sort: out of memory\n");
            exit();
          }
          strcpy(p, line_buf);
          lines[num_lines++] = p;
        }
        line_len = 0;
      } else {
        if (line_len < MAX_LINE_LEN - 1) {
          line_buf[line_len++] = c;
        }
      }
    }
  }

  if (n < 0) {
    printf(2, "sort: read error\n");
    exit();
  }

  // Handle last line if file did not end with a newline
  if (line_len > 0 && num_lines < MAX_LINES) {
    line_buf[line_len] = '\0';
    char *p = malloc(line_len + 1);
    if (p == 0) {
      printf(2, "sort: out of memory\n");
      exit();
    }
    strcpy(p, line_buf);
    lines[num_lines++] = p;
  }
}

void
sort_lines(void)
{
  int i, j;
  for (i = 1; i < num_lines; i++) {
    char *key = lines[i];
    j = i - 1;
    while (j >= 0 && compare_lines(lines[j], key) > 0) {
      lines[j + 1] = lines[j];
      j--;
    }
    lines[j + 1] = key;
  }
}

int
main(int argc, char *argv[])
{
  int i, j;
  int file_count = 0;

  // Parse flags
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] != '\0') {
      for (j = 1; argv[i][j] != '\0'; j++) {
        if (argv[i][j] == 'r') {
          flag_r = 1;
        } else if (argv[i][j] == 'n') {
          flag_n = 1;
        } else if (argv[i][j] == 'u') {
          flag_u = 1;
        } else {
          printf(2, "sort: invalid option -%c\n", argv[i][j]);
          exit();
        }
      }
    } else {
      file_count++;
    }
  }

  // Read from standard input if no files specified
  if (file_count == 0) {
    read_lines(0);
  } else {
    for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-' && argv[i][1] != '\0')
        continue;
      int fd = open(argv[i], 0);
      if (fd < 0) {
        printf(2, "sort: cannot open %s\n", argv[i]);
        exit();
      }
      read_lines(fd);
      close(fd);
    }
  }

  // Sort the collected lines
  sort_lines();

  // Print sorted lines (applying -u if enabled)
  for (i = 0; i < num_lines; i++) {
    if (flag_u && i > 0 && is_duplicate(lines[i], lines[i - 1])) {
      continue;
    }
    printf(1, "%s\n", lines[i]);
  }

  // Free allocated memory
  for (i = 0; i < num_lines; i++) {
    free(lines[i]);
  }

  exit();
}

