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
	context_uload(&pcb[0], "/bin/hello");
	pcb[0].nice = 1;
	context_uload(&pcb[1], "/bin/pal");
	pcb[1].nice = 10;
	switch_boot_pcb();
}

_Context* schedule(_Context *prev) {
	current->cp = prev;
	//current = (current == &pcb[1] ? &pcb[0] : &pcb[1]);
	int i = 0;
	bool needSchedule = false;
	if(current == &pcb_boot) needSchedule = true;
	for(; i < MAX_NR_PROC; i++){
		//Log("i:%d, run:%d, nice:%d", i, pcb[i].run, pcb[i].nice);
		if(current == &pcb[i]){
			if(pcb[i].run && pcb[i].run % pcb[i].nice == 0){
				needSchedule = true;
			}
			pcb[i].run++;
			break;
		}
	}
	if(needSchedule){
		for(int j = 1; j <= MAX_NR_PROC; j++){
			int idx = (i + j) % MAX_NR_PROC;
			if(pcb[idx].nice){
				current = &pcb[idx];
				break;
			}
		}
	}
	return current->cp;
}
