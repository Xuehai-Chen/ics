#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
	int fd = fs_open(filename, 0, 0);
	size_t sz = fs_filesz(fd);
	fs_lseek(fd, 0, SEEK_SET);
	int count = (sz + PGSIZE - 1) / PGSIZE;
	for(int i = 0; i < count; i++){
		void* pa = new_page();
		_map(as, DEFAULT_ENTRY + i * PGSIZE, pa);
		size_t cp_size = PGSIZE;
		if(i == count -1){
			cp_size = sz - (count - 1) * PGSIZE;
		}
		//:printf("pa: %p, cp_size: %d\n", pa, cp_size);
		fs_read(fd, pa, cp_size);
	}

	return (uintptr_t)DEFAULT_ENTRY;
}
