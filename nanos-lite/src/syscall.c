#include "common.h"
#include "syscall.h"
#include "fs.h"

void sys_write(_RegSet *r){
	int i;
	//Log("count:%d", SYSCALL_ARG4(r));	
	switch (SYSCALL_ARG2(r)){
		case 1:
		case 2:
			for(i = 0; i < SYSCALL_ARG4(r); i++){
				_putc(*((char *)SYSCALL_ARG3(r) + i));
			}
			SYSCALL_ARG1(r) = i;
			break;
		default:
			SYSCALL_ARG1(r) = -1;
	}
}

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
			SYSCALL_ARG1(r) = fs_read(SYSCALL_ARG2(r), (void*)SYSCALL_ARG3(r), SYSCALL_ARG4(r));
			break;
		case SYS_write:
			//Log("cout:%d", SYSCALL_ARG4(r));
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
