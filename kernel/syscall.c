/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "elf.h"
#include "spike_interface/spike_utils.h"
//@lab1_challenge1: add for fetch the section .symtab and .strtab
extern elf_ctx elfloader;
//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  sprint(buf);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process).
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}
int get_symbol_index(uint64 ra){

  int idx = -1;
  for (int i = 0; i < elfloader.sym_num; ++i) {
    // if ra is between the entry address of symbol and the end address of symbol, this is the corresponding function
    if (elfloader.symbols[i].st_value < ra && elfloader.symbols[i].st_value + elfloader.symbols[i].st_size > ra) {
      idx = i;
    }
  }
  // sprint("index: %d\n",idx);
  return idx;
}
//@lab1_challenge1: impelement the SYS_user_print_backtrace
ssize_t sys_user_print_backtrace(int depth){
  // uint64 sp = current->trapframe->regs.sp + 40;
  uint64 fp = current->trapframe->regs.s0;// according to the lab doc, s0 stores fp
  /*  from the picture of stack frame we know that `fp - 8` is the address of leaf function
  (print_backtrace)'s fp, and it stores the stack bottom address of the function that calls it(f8),
  so we can get the `ra` from the bottom address of its function.
  */
  uint64 ra = *(uint64 *)(fp-8) - 8;
  // sprint("ra: %lx\n",*(uint64 *)ra);
  int index = 0;
  int dep = 0;

  for(;dep < depth;dep++,ra=*(uint64 *)(ra-8)-8){
    // uint64 _sp = *(uint64 *)sp;//the return address of function
    index = get_symbol_index(*(uint64 *)ra);

    if(index == -1)
    {
      sprint("fail to backtrace symbol %lx\n", *(uint64 *)ra);
      continue;
    }
    // sprint("index: %d\n",index);
    sprint("%s\n",&elfloader.strtab[elfloader.symbols[index].st_name]);
  }
  return 0;
}
//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    case SYS_user_print_backtrace:
      return sys_user_print_backtrace(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
