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
#include "sched.h"

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
  // reclaim the current process, and reschedule. added @lab3_1
  free_process( current );
  schedule();
  return 0;
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

//
// kerenl entry point of naive_fork
//
ssize_t sys_user_fork() {
  sprint("User call fork.\n");
  return do_fork( current );
}

//
// kerenl entry point of yield. added @lab3_2
//
ssize_t sys_user_yield() {
  // TODO (lab3_2): implment the syscall of yield.
  // hint: the functionality of yield is to give up the processor. therefore,
  // we should set the status of currently running process to READY, insert it in
  // the rear of ready queue, and finally, schedule a READY process to run.
  current->status = READY;
  insert_to_ready_queue(current);
  schedule();

  return 0;
}

// @lab3_challenge2
semaphore sems[20];
ssize_t sys_user_sem_new(int num){
  // find a semaphore that hasn't been used
  for(int i = 0;i < 20;i++){
    if(!sems[i].used){
      sems[i].used = 1;
      sems[i].value = num;
      sems[i].wait_queue_head = 0;
      return i;
    }
  }
  return -1;
}

ssize_t sys_user_sem_P(int semap){
  // sprint("-----------semap: %d call P----------\n",semap);
  if(semap < 0 || semap >= 20)
    return -1;
  sems[semap].value--;
  if(sems[semap].value < 0){//which means that this process need to insert into wait_queue
    insert_to_wait_queue(semap, current);
    // sprint("wait_queue_head: %lx\n",sems[semap].wait_queue_head);
    // current->status = BLOCKED;
    schedule();
  }
  return 0;
}

ssize_t sys_user_sem_V(int semap){
  // sprint("-----------semap: %d call V----------\n",semap);
  if(semap < 0 || semap >= 20)
    return -1;
  // sprint("v oprator is called\n");
  sems[semap].value++;
  // sprint("sems[semap].value: %d\n",sems[semap].value);
  // sprint("semap: %d sems[semap].wait_queue_head: %lx\n",semap, sems[semap].wait_queue_head);
  if(sems[semap].wait_queue_head){//if there is process in the wait_queue
    // sprint("semaphore %d has process in its wait_queue\n",semap);
    process *p = sems[semap].wait_queue_head;
    sems[semap].wait_queue_head = sems[semap].wait_queue_head -> queue_next;
    insert_to_ready_queue(p);
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
    // added @lab2_2
    case SYS_user_allocate_page:
      return sys_user_allocate_page();
    case SYS_user_free_page:
      return sys_user_free_page(a1);
    case SYS_user_fork:
      return sys_user_fork();
    case SYS_user_yield:
      return sys_user_yield();
    case SYS_user_sem_new:
      return sys_user_sem_new(a1);
    case SYS_user_sem_P:
      return sys_user_sem_P(a1);
    case SYS_user_sem_V:
      return sys_user_sem_V(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
