#include "common.h"
#include "syscall.h"
#include "proc.h"

int fs_open(const char*, int, int);
ssize_t fs_read(int, void*, size_t);
size_t fs_write(int fd, const void* buf, size_t len);
off_t fs_lseek(int, off_t, int);
int fs_close(int);
void naive_uload(PCB*, const char*);

_Context* do_syscall(_Context *c) {
	uintptr_t a[4];
	a[0] = c->GPR1;

	switch (a[0]) {
		case SYS_yield:
			_yield();
			c->GPR1 = 0;
			break;
		case SYS_open:
			c->GPR1 = fs_open((void*)c->GPR2, c->GPR3, c->GPR4);
			break;
		case SYS_read:
			c->GPR1 = fs_read(c->GPR2, (void*)c->GPR3, c->GPR4);
			break;
		case SYS_write:
			c->GPR1 = fs_write(c->GPR2, (void*)c->GPR3, c->GPR4);
			break;
		case SYS_close:
			c->GPR1 = fs_close(c->GPR2);
			break;
		case SYS_lseek:
			c->GPR1 = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
			break;
		case SYS_brk:
			c->GPR1 = 0;
			break;
		case SYS_exit:
			c->GPR2 = (intptr_t)&"/bin/init";
		case SYS_execve:
			naive_uload(NULL, (char*)c->GPR2);
			break;
		default: panic("Unhandled syscall ID = %d", a[0]);
	}

	return NULL;
}
