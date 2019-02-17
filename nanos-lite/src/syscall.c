#include "common.h"
#include "syscall.h"
#include "fs.h"

_RegSet* do_syscall(_RegSet *r) {
	uintptr_t a[4];
	a[0] = SYSCALL_ARG1(r);

	switch (a[0]) {
		case SYS_none:
			SYSCALL_ARG1(r) = 1;
			break;
		case SYS_exit:
			_halt(SYSCALL_ARG2(r));
			break;
		case SYS_open:
			SYSCALL_ARG1(r) = fs_open((void*)SYSCALL_ARG2(r), 0, 0);
			break;
		case SYS_lseek:
			SYSCALL_ARG1(r) = fs_lseek(SYSCALL_ARG2(r), SYSCALL_ARG3(r), SYSCALL_ARG4(r));
			break;
		case SYS_read:
			//Log("fd:%d, len:%d", SYSCALL_ARG2(r), SYSCALL_ARG4(r));
			SYSCALL_ARG1(r) = fs_read(SYSCALL_ARG2(r), (void*)SYSCALL_ARG3(r), SYSCALL_ARG4(r));
			break;
		case SYS_write:
			SYSCALL_ARG1(r) = fs_write(SYSCALL_ARG2(r), (void*)SYSCALL_ARG3(r), SYSCALL_ARG4(r));
			break;
		case SYS_close:
			SYSCALL_ARG1(r) = fs_close(SYSCALL_ARG2(r));
			break;
		case SYS_brk:
			SYSCALL_ARG1(r) = 0;
			break;
		default: panic("Unhandled syscall ID = %d", a[0]);
	}

	return NULL;
}
