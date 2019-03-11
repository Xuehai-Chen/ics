#include "proc.h"

#define DEFAULT_ENTRY 0x4000000

size_t ramdisk_read(void*, size_t, size_t);
size_t ramdisk_write(const void*, size_t, size_t);
size_t get_ramdisk_size();

size_t fs_filesz(int);
int fs_open(const char*, int, int);
ssize_t fs_read(int, void*, size_t);
ssize_t fs_write(int, const void*, size_t);
off_t fs_lseek(int, off_t, int);
int fs_close(int);

static uintptr_t loader(PCB *pcb, const char *filename) {
	int fd = fs_open(filename, 0, 0);
	fs_read(fd, (void *)DEFAULT_ENTRY, fs_filesz(fd));
	return DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
	uintptr_t entry = loader(pcb, filename);
	((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
	_Area stack;
	stack.start = pcb->stack;
	stack.end = stack.start + sizeof(pcb->stack);

	pcb->tf = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
	uintptr_t entry = loader(pcb, filename);

	_Area stack;
	stack.start = pcb->stack;
	stack.end = stack.start + sizeof(pcb->stack);

	pcb->tf = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
