/*
 * elf.h
 *
 *  Created on: May 18, 2013
 *      Author: Bernhard
 */

#ifndef ELF_H_
#define ELF_H_

#include <stdlib.h>
#include <inttypes.h>
#include "binary.h"

binary_t* elf_init(void* ident, binary_read read_function);
void elf_close(binary_t* elf);

#define EI_MAG0			0
#define EI_MAG1			1
#define EI_MAG2			2
#define EI_MAG3			3
#define EI_CLASS		4
#define EI_DATA			5
#define EI_VERSION		6
#define EI_OSABI		7
#define EI_ABIVERSION	8
#define EI_PAD			9
#define EI_NIDENT       16

#define ELFCLASSNONE	0 /* invalid class */
#define ELFCLASS32		1 /* 32-bit objects */
#define ELFCLASS64		2 /* 64-bit objects */

#define ELFDATANONE		0 /* invalid dataencoding */
#define ELFDATA2LSB		1
#define ELFDATA2MSB		2

#define ET_NONE			0 /* No file type */
#define ET_REL 			1 /* Relocatable file */
#define ET_EXEC 		2 /* Executable file */
#define ET_DYN 			3 /* Shared object file */
#define ET_CORE 		4 /* Core file */
#define ET_LOPROC 	0xff00 /* Processor-specific */
#define ET_HIPROC 	0xffff /* Processor-specific */

#define EM_ARM		0x28 /* arm */

#define PT_LOAD		1

#define PF_X		0x1 /* execute */
#define PF_W		0x2 /* write */
#define PF_R		0x4 /* read */

#if 1 /* TODO: check for 32bit architecture */
typedef void* Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef int Elf32_Sword;
typedef unsigned int Elf32_Word;

typedef struct {
        unsigned char   e_ident[EI_NIDENT];
        Elf32_Half      e_type;
        Elf32_Half      e_machine;
        Elf32_Word      e_version;
        Elf32_Addr      e_entry;
        Elf32_Off       e_phoff;
        Elf32_Off       e_shoff;
        Elf32_Word      e_flags;
        Elf32_Half      e_ehsize;
        Elf32_Half      e_phentsize;
        Elf32_Half      e_phnum;
        Elf32_Half      e_shentsize;
        Elf32_Half      e_shnum;
        Elf32_Half      e_shstrndx;
} Elf32_Ehdr;

typedef struct {
        Elf32_Word      p_type;
        Elf32_Off       p_offset;
        Elf32_Addr      p_vaddr;
        Elf32_Addr      p_paddr;
        Elf32_Word      p_filesz;
        Elf32_Word      p_memsz;
        Elf32_Word      p_flags;
        Elf32_Word      p_align;
} Elf32_Phdr;

/* TODO: check for 32- or 64-bit architecture */
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#define ELFCLASS ELFCLASS32

#endif /* 32bit architecture */

#if 0 /* 64bit architecture */
/* TODO: http://docs.oracle.com/cd/E23824_01/html/819-0690/chapter7-6.html#chapter6-tbl-7-1 Table 12-2 */
typedef struct {
        unsigned char   e_ident[EI_NIDENT];
        Elf64_Half      e_type;
        Elf64_Half      e_machine;
        Elf64_Word      e_version;
        Elf64_Addr      e_entry;
        Elf64_Off       e_phoff;
        Elf64_Off       e_shoff;
        Elf64_Word      e_flags;
        Elf64_Half      e_ehsize;
        Elf64_Half      e_phentsize;
        Elf64_Half      e_phnum;
        Elf64_Half      e_shentsize;
        Elf64_Half      e_shnum;
        Elf64_Half      e_shstrndx;
} Elf64_Ehdr;


typedef struct {
        Elf64_Word      p_type;
        Elf64_Word      p_flags;
        Elf64_Off       p_offset;
        Elf64_Addr      p_vaddr;
        Elf64_Addr      p_paddr;
        Elf64_Xword     p_filesz;
        Elf64_Xword     p_memsz;
        Elf64_Xword     p_align;
} Elf64_Phdr;
#endif /* 64bit architecture */

/* TODO: define architecture depended */
#define ELFDATA ELFDATA2LSB
#define ELFMACHINE EM_ARM


#endif /* ELF_H_ */
