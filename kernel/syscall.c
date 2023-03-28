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
#include "pmm.h"
#include "vmm.h"
#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  // buf is now an address in user space of the given app's user stack,
  // so we have to transfer it into phisical address (kernel is running in direct mapping).
  assert( current );
  char* pa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)buf);
  sprint(pa);
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

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page() {
  void* pa = alloc_page();
  uint64 va = g_ufree_page;
  g_ufree_page += PGSIZE;
  user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
         prot_to_type(PROT_WRITE | PROT_READ, 1));

  return va;
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  user_vm_unmap((pagetable_t)current->pagetable, va, PGSIZE, 1);
  return 0;
}
//---------------------------------------------------------------------------------------------
// uint64 add_brk(int size){
//   char *mem;
//   for(int i = current->brk;i < current->brk + size;i += PGSIZE){
//     mem = (char *)alloc_page();
//     if(mem == 0)
//       panic("fail to add_brk");
//     memset(mem, 0, PGSIZE);
//     map_pages(current->pagetable, current->brk, PGSIZE, (uint64)mem, prot_to_type(PROT_READ | PROT_WRITE,1));
//   }
//   current->brk += size;
//   return current->brk;
// }
// uint64 sys_user_better_malloc(int n){
//   int idx = 0;
//   for(idx = current->head;idx < 1024;idx = current->blocks[idx].next){
//     if(current->blocks[idx].size >= n && current->blocks[idx].occupied == 0){
//       current->blocks[idx].occupied = 1;
//       return current->blocks[idx].offset;
//     }
//     if(current->blocks[idx].next == current->tail)
//       break;
//   }
//   //  if you can't find a emtpy block to be allocated, then we need to get one more page
//   uint64 brk = current->brk;
//   add_brk(brk);
//   current->blocks[idx].occupied = 1;
//   current->blocks[idx].offset = brk;
//   current->blocks[idx].size = n;
//   current->blocks[idx].next = idx + 1;
//   return brk;
// }

// uint64 sys_user_better_free(uint64 address){
//   for(int i = current->head;i < 1024;i = current->blocks[i].next)
//   {
//     if(current->blocks[i].offset == address)
//     {
//       current->blocks[i].occupied = 0;
//     }
//   }
//   return 0;
// }
//--------------------------------------------------------------------------------------------
//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
uint64 sys_user_better_malloc(uint64 n){
  return better_malloc(n);
}
uint64 sys_user_better_free(uint64 va) {
  better_free((void *)va);
  return 0;
}

long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    // added @lab2_2
    case SYS_user_allocate_page:
      return sys_user_allocate_page();
    case SYS_user_free_page:
      return sys_user_free_page(a1);
    case SYS_user_better_malloc:
      return sys_user_better_malloc(a1);
    case SYS_user_better_free:
      return sys_user_better_free(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
