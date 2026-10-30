// Snapshot of one process, shared between kernel and user space (pstree).
struct pinfo {
  int pid;
  int ppid;        // 0 for init
  uint64 sz;       // bytes of user memory
  char name[16];
};
