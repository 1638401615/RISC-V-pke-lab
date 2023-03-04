#include "kernel/riscv.h"
#include "kernel/process.h"
#include "spike_interface/spike_utils.h"
#include "stdio.h"
#include "string.h"
static void handle_instruction_access_fault() { panic("Instruction access fault!"); }

static void handle_load_access_fault() { panic("Load access fault!"); }

static void handle_store_access_fault() { panic("Store/AMO access fault!"); }

static void handle_illegal_instruction() { panic("Illegal instruction!"); }

static void handle_misaligned_load() { panic("Misaligned Load!"); }

static void handle_misaligned_store() { panic("Misaligned AMO!"); }



void print_line_information(addr_line line){
  char path[100];
  // find the dir path of code
  strcpy(path, current->dir[current->file[line.file].dir]);
  int dir_len = strlen(current->dir[current->file[line.file].dir]);
  // add the file name of the code
  path[dir_len] = '/';
  strcpy(path + dir_len + 1, current->file[line.file].file);

  // find the source code of this line
  struct stat file_stat;
  spike_file_t *fp = spike_file_open(path, O_RDONLY, 0);
  // in order to get the length of the file, we need to load the file statement struct
  spike_file_stat(fp, &file_stat);
  char context[10240];
  // get the source code context from the file pointer
  spike_file_read(fp, context, file_stat.st_size);
  spike_file_close(fp);

  // find the line which call illegal instruction
  int line_number = 1;


  // sprint("call  print_error\n");
  for(int i = 0;i < file_stat.st_size;i++){
    int line_start = i;
    while(i < file_stat.st_size && context[i] != '\n')
      i++;

    if(line_number == line.line)
    {
      // sprint("line: %d\n",line->line);
      // sprint("%d\n",line_number);
      context[i] = '\0';
      sprint("Runtime error at %s:%d\n%s\n",path, line.line, context+line_start);
      break;
    }
    line_number++;
  }
}
//@lab1_challenge2 add a function to print the information about the illegal instruction
void print_error(){
  // the address of illegal instruction is stored in epc register.
  uint64 mepc = read_csr(mepc);
  // sprint("mepc: %lx\n",mepc);
  for(int i = 0;i < current->line_ind;i++){
    //traverse the address of all the code to find out the illegal instruction
    // sprint("%lx\n",current->line[i].addr);
    if(mepc == current->line[i].addr){
      print_line_information(current->line[i]);
      break;
    }
  }
}

// added @lab1_3
static void handle_timer() {
  int cpuid = 0;
  // setup the timer fired at next time (TIMER_INTERVAL from now)
  *(uint64*)CLINT_MTIMECMP(cpuid) = *(uint64*)CLINT_MTIMECMP(cpuid) + TIMER_INTERVAL;

  // setup a soft interrupt in sip (S-mode Interrupt Pending) to be handled in S-mode
  write_csr(sip, SIP_SSIP);
}

//
// handle_mtrap calls a handling function according to the type of a machine mode interrupt (trap).
//
void handle_mtrap() {
  uint64 mcause = read_csr(mcause);
  switch (mcause) {
    case CAUSE_MTIMER:
      handle_timer();
      break;
    case CAUSE_FETCH_ACCESS:
      handle_instruction_access_fault();
      break;
    case CAUSE_LOAD_ACCESS:
      handle_load_access_fault();
    case CAUSE_STORE_ACCESS:
      handle_store_access_fault();
      break;
    case CAUSE_ILLEGAL_INSTRUCTION:
      // TODO (lab1_2): call handle_illegal_instruction to implement illegal instruction
      // interception, and finish lab1_2.
      // @lab1_challenge2 add a print_error function to show the information about illegal instruction
      print_error();
      handle_illegal_instruction();

      break;
    case CAUSE_MISALIGNED_LOAD:
      handle_misaligned_load();
      break;
    case CAUSE_MISALIGNED_STORE:
      handle_misaligned_store();
      break;

    default:
      sprint("machine trap(): unexpected mscause %p\n", mcause);
      sprint("            mepc=%p mtval=%p\n", read_csr(mepc), read_csr(mtval));
      panic( "unexpected exception happened in M-mode.\n" );
      break;
  }
}
