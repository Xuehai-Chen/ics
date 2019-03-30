#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used));
static PCB pcb_boot;
PCB *current;

void naive_uload(PCB*, const char*);
void context_kload(PCB*, void*);
void context_uload(PCB*, char*);

void switch_boot_pcb() {
	current = &pcb_boot;
}

void hello_fun(void *arg) {
	int j = 1;
	while (1) {
		Log("Hello World from Nanos-lite for the %dth time!", j);
		j ++;
		_yield();
	}
}

void init_proc() {
	//naive_uload(NULL, "/bin/init");
	context_kload(&pcb[0], (void*)hello_fun);
	context_uload(&pcb[1], "/bin/init");
	switch_boot_pcb();
}

_Context* schedule(_Context *prev) {
	current->cp = prev;
	current = (current == &pcb[1] ? &pcb[0] : &pcb[1]);
	//current = &pcb[1];
	return current->cp;
}
