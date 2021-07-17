#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <string.h>

void
elf_test(int fd)
{
	Elf32_Ehdr ehdr;
	Elf32_Off  shstrtab_off;
	Elf32_Shdr shstrtab;
	Elf32_Off  strtab_off;
	Elf32_Half strtab_size;
	Elf32_Off  dynstr_off;
	Elf32_Half dynstr_size;
	Elf32_Off  symtab_off;
	Elf32_Half symtab_entsize;
	Elf32_Half symtab_num;
	Elf32_Off  dynsym_off;
	Elf32_Half dynsym_entsize;
	Elf32_Half dynsym_num;
	Elf32_Half i;

	read(fd, &ehdr, sizeof(ehdr));
	printf("[*] Magic Bytes: %hhX %c%c%c\n", ehdr.e_ident[EI_MAG0],
						 ehdr.e_ident[EI_MAG1],
						 ehdr.e_ident[EI_MAG2],
						 ehdr.e_ident[EI_MAG3]);
	printf("[*] Bits: %d\n", ehdr.e_ident[EI_CLASS] * 32);
	printf("[*] Section Header Items: %d\n", ehdr.e_shnum);
	printf("[*] String Table Index: %d\n", ehdr.e_shstrndx);
	shstrtab_off = ehdr.e_shoff + (ehdr.e_shstrndx * ehdr.e_shentsize);
	printf("[*] String Table Offset: %u\n", shstrtab_off);

	lseek(fd, shstrtab_off, SEEK_SET);
	read(fd, &shstrtab, ehdr.e_shentsize);
	printf("[*] String Table Type: %d\n", shstrtab.sh_type);
	printf("====================\n");
	shstrtab_off = shstrtab.sh_offset;
	lseek(fd, ehdr.e_shoff, SEEK_SET);
	printf("[*] SHDR:\n");
	for (i = 0; i < ehdr.e_shnum; ++i) {
		Elf32_Shdr shdr;
		char str[64] = { 0 };

		read(fd, &shdr, ehdr.e_shentsize);
		pread(fd, str, sizeof(str) - 1, shstrtab_off + shdr.sh_name);
		printf("[*] Section Header Index: %d\n", i);
		printf("[*] Section Header String: %s\n", str);
		printf("--------------------\n");

		if (!strcmp(str, ".strtab")) {
			strtab_off = shdr.sh_offset;
			strtab_size = shdr.sh_size;
		} else if (!strcmp(str, ".dynstr")) {
			dynstr_off = shdr.sh_offset;
			dynstr_size = shdr.sh_size;
		} else if (!strcmp(str, ".symtab")) {
			symtab_off = shdr.sh_offset;
			symtab_entsize = shdr.sh_entsize;
			symtab_num = shdr.sh_size / shdr.sh_entsize;
		} else if (!strcmp(str, ".dynsym")) {
			dynsym_off = shdr.sh_offset;
			dynsym_entsize = shdr.sh_entsize;
			dynsym_num = shdr.sh_size / shdr.sh_entsize;
		}
	}

	printf("[*] .strtab Offset:  %u\n", strtab_off);
	printf("[*] .strtab Size:    %u\n", strtab_size);

	printf("[*] .dynstr Offset:  %u\n", dynstr_off);
	printf("[*] .dynstr Size:    %u\n", dynstr_size);

	printf("[*] .symtab Offset:  %u\n", symtab_off);
	printf("[*] .symtab EntSize: %u\n", symtab_entsize);
	printf("[*] .symtab Num:     %u\n", symtab_num);

	printf("[*] .dynsym Offset:  %u\n", dynsym_off);
	printf("[*] .dynsym EntSize: %u\n", dynsym_entsize);
	printf("[*] .dynsym Num:     %u\n", dynsym_num);
	printf("====================\n");
	lseek(fd, symtab_off, SEEK_SET);
	printf("[*] SYMTAB:");
	for (i = 0; i < symtab_num; ++i) {
		Elf32_Sym sym;
		char symname[64] = { 0 };

		read(fd, &sym, symtab_entsize);
		pread(fd, symname, sizeof(symname), strtab_off + sym.st_name);

		printf("[*] Symbol Name: %s\n", symname);
		printf("[*] Symbol Addr: %p\n", (void *)sym.st_value);
		printf("--------------------\n");
	}
	printf("====================\n");
	lseek(fd, dynsym_off, SEEK_SET);
	printf("[*] DYNSYM:");
	for (i = 0; i < dynsym_num; ++i) {
		Elf32_Sym sym;
		char symname[64] = { 0 };

		read(fd, &sym, dynsym_entsize);
		pread(fd, symname, sizeof(symname), dynstr_off + sym.st_name);

		printf("[*] Symbol Name: %s\n", symname);
		printf("[*] Symbol Addr: %p\n", (void *)sym.st_value);
	}
	printf("====================\n");
}

int
main()
{
	int fd;
	fd = open(OUTPUT, O_RDONLY);

	if (fd == -1) {
		printf("[!] Unable to open ELF file!\n");
		return -1;
	}

	elf_test(fd);

	close(fd);

	return 0;
}