#include "proc.h"
static void *pf = NULL;

void* new_page(size_t nr_page) {
	void *p = pf;
	pf += PGSIZE * nr_page;
	assert(pf < (void *)_heap.end);
	return p;
}

void free_page(void *p) {
	panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t new_brk) {
	//Log("new_brk:%d\tcurrent->cur_brk:%d\tcurrent->max_brk:%d", new_brk, current->cur_brk, current->max_brk);
	if(current->max_brk == 0){
		current->max_brk = (new_brk & 0xfff) ? ((new_brk & ~0xfff) + PGSIZE) : new_brk;
	}else if(new_brk > current->max_brk){
		int count = (new_brk + PGSIZE - 1 - current->max_brk) / PGSIZE;
		for(int i = 0; i < count; i++){
			void *pa = new_page(1);
			_map(&current->as, (void*)(current->max_brk + i*PGSIZE), pa, 0);
		}
		current->max_brk = (new_brk & 0xfff) ? ((new_brk & ~0xfff) + PGSIZE) : new_brk;
	}
	current->cur_brk = new_brk;
	return 0;
}

void init_mm() {
	pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
	Log("free physical pages starting from %d", pf);

	_vme_init(new_page, free_page);
}
