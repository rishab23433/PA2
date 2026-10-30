// pstree [-p] [-m]: print the process tree rooted at init.
//   -p  show PIDs as name(pid)
//   -m  show memory size as name [NNNB]
// Flags may be given separately (-p -m) or combined (-pm / -mp).
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

static struct pinfo procs[NPROC];
static int nprocs;
static int show_pid, show_mem;

static void
print_node(struct pinfo *p, int depth)
{
  if (depth > 0) {
    for (int i = 0; i < 4 * (depth - 1) + 2; i++)
      printf(" ");
    printf("|- ");
  }
  printf("%s", p->name);
  if (show_pid)
    printf("(%d)", p->pid);
  if (show_mem)
    printf(" [%dB]", (int)p->sz);
  printf("\n");
}

static void
walk(int idx, int depth)
{
  print_node(&procs[idx], depth);
  if (depth > NPROC)            // safety against a corrupted snapshot
    return;
  for (int i = 0; i < nprocs; i++) {
    if (i != idx && procs[i].ppid == procs[idx].pid)
      walk(i, depth + 1);
  }
}

int
main(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    char *a = argv[i];
    if (a[0] != '-' || a[1] == 0) {
      fprintf(2, "usage: pstree [-p] [-m]\n");
      exit(1);
    }
    for (int j = 1; a[j]; j++) {
      if (a[j] == 'p')
        show_pid = 1;
      else if (a[j] == 'm')
        show_mem = 1;
      else {
        fprintf(2, "pstree: unknown flag -%c\nusage: pstree [-p] [-m]\n", a[j]);
        exit(1);
      }
    }
  }

  nprocs = getprocs(procs, NPROC);
  if (nprocs < 0) {
    fprintf(2, "pstree: getprocs failed\n");
    exit(1);
  }

  for (int i = 0; i < nprocs; i++) {
    if (procs[i].pid == 1) {
      walk(i, 0);
      exit(0);
    }
  }
  fprintf(2, "pstree: init not found\n");
  exit(1);
}
