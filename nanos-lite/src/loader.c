#include "proc.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void*)0x8048000)

size_t ramdisk_read(void*, size_t, size_t);
size_t ramdisk_write(const void*, size_t, size_t);
size_t get_ramdisk_size();

size_t fs_filesz(int);
int fs_open(const char*, int, int);
ssize_t fs_read(int, void*, size_t);
ssize_t fs_write(int, const void*, size_t);
off_t fs_lseek(int, off_t, int);
int fs_close(int);
void* new_page(size_t);

static uintptr_t loader(PCB *pcb, const char *filename) {
	Log("filename:%s", filename);
	int fd = fs_open(filename, 0, 0);
	size_t sz = fs_filesz(fd);
	fs_lseek(fd, 0, SEEK_SET);
	int count = (sz + PGSIZE - 1) / PGSIZE;
	for(int i = 0; i < count; i++){
		void* pa = new_page(1);
		_map(&pcb->as, DEFAULT_ENTRY + i * PGSIZE, pa, 0);
		size_t cp_size = PGSIZE;
		if(i == count -1){
			cp_size = sz - (count - 1) * PGSIZE;
		}
		//Log("va: %d, pa: %d, cp_size: %d",DEFAULT_ENTRY + i * PGSIZE,  pa, cp_size);
		fs_read(fd, pa, cp_size);
	}
	pcb->max_brk = pcb->cur_brk = (uintptr_t)DEFAULT_ENTRY + count * PGSIZE;
	return (uintptr_t)DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
	uintptr_t entry = loader(pcb, filename);
	((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
	_Area stack;
	stack.start = pcb->stack;
	stack.end = stack.start + sizeof(pcb->stack);

	pcb->cp = _kcontext(stack, entry, NULL);
	printf("stack.end:%d\n",stack.end);
	printf("stack.start:%d\n", stack.start);
	printf("pcb->cp:%d\n", pcb->cp);
}

void context_uload(PCB *pcb, const char *filename) {
	_protect(&pcb->as);
	uintptr_t entry = loader(pcb, filename);

	_Area stack;
	stack.start = pcb->stack;
	stack.end = stack.start + sizeof(pcb->stack);

	pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
	printf("ustack.end:%d\n",stack.end);
	printf("ustack.start:%d\n", stack.start);
	printf("upcb->cp:%d\n", pcb->cp);
}
