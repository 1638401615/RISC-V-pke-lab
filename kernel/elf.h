#ifndef _ELF_H_
#define _ELF_H_

#include "util/types.h"
#include "process.h"

#define MAX_CMDLINE_ARGS 64
//@lab1_challenge1: elf section header
typedef struct elf_sect_header_t{
    uint32 name;      /* Section name (string tbl index) */
    uint32 type;
    uint64 flags;
    uint64 addr;
    uint64 offset;    /* Section offset*/
    uint64 size;      /* Section size in bytes */
    uint32 link;
    uint32 info;
    uint64 addralign;
    uint64 entsize;
} elf_sect_header;

//@lab1_challenge1: elf symbolic section
typedef struct elf_symbol_section_t{
    uint32 st_name;           //  the index of string table which stores the name of the symbol
    unsigned char st_info;    //  the type and other attributes of symbol
    unsigned char st_other;
    uint16 st_shndx;          //  the index of the section
    uint64 st_value;          //  if symbol is function, value is the entry address of the function
    uint64 st_size;           //  the size of symbol

} elf_symbol_sect;
// elf header structure
typedef struct elf_header_t {
  uint32 magic;
  uint8 elf[12];
  uint16 type;      /* Object file type */
  uint16 machine;   /* Architecture */
  uint32 version;   /* Object file version */
  uint64 entry;     /* Entry point virtual address */
  uint64 phoff;     /* Program header table file offset */
  uint64 shoff;     /* Section header table file offset */
  uint32 flags;     /* Processor-specific flags */
  uint16 ehsize;    /* ELF header size in bytes */
  uint16 phentsize; /* Program header table entry size */
  uint16 phnum;     /* Program header table entry count */
  uint16 shentsize; /* Section header table entry size */
  uint16 shnum;     /* Section header table entry count */
  uint16 shstrndx;  /* Section header string table index */
} elf_header;

// Program segment header.
typedef struct elf_prog_header_t {
  uint32 type;   /* Segment type */
  uint32 flags;  /* Segment flags */
  uint64 off;    /* Segment file offset */
  uint64 vaddr;  /* Segment virtual address */
  uint64 paddr;  /* Segment physical address */
  uint64 filesz; /* Segment size in file */
  uint64 memsz;  /* Segment size in memory */
  uint64 align;  /* Segment alignment */
} elf_prog_header;

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian
#define ELF_PROG_LOAD 1
#define SHT_SYMTAB	  2		/* Symbol table */
#define SHT_STRTAB	  3		/* String table */
#define STT_FUNC 2        /* symbol type is function*/

typedef enum elf_status_t {
  EL_OK = 0,

  EL_EIO,
  EL_ENOMEM,
  EL_NOTELF,
  EL_ERR,

} elf_status;

typedef struct elf_ctx_t {
  void *info;
  elf_header ehdr;

  // to get the name of each function called, we need to load .symtab and .strtab into the context

  elf_symbol_sect symbols[1024];
  unsigned char strtab[1024];
  uint32 sym_num;
} elf_ctx;

elf_status elf_init(elf_ctx *ctx, void *info);
elf_status elf_load(elf_ctx *ctx);

// @lab1_challenge1 added for load .symtab and .strtab into memory
elf_status elf_load_symbol(elf_ctx *ctx);
void load_bincode_from_host_elf(process *p);

#endif
