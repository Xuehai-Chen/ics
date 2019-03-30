#include <am.h>
#include <x86.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void vectrap();
void vecnull();
void vecsys();
void irq0();

_Context* irq_handle(_Context *cp) {
	_Context *next = cp;
	if (user_handler) {
		_Event ev = {0};
		switch (cp->irq) {
			case 0x80:
				ev.event = _EVENT_SYSCALL;
				break;
			case 0x81:
				ev.event = _EVENT_YIELD;
				break;
			default: ev.event = _EVENT_ERROR; break;
		}

		next = user_handler(ev, cp);
		if (next == NULL) {
			next = cp;
		}
	}

	return next;
}

static GateDesc idt[NR_IRQ];

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
	// initialize IDT
	for (unsigned int i = 0; i < NR_IRQ; i ++) {
		idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
	}

	// -------------------- system call --------------------------
	idt[0x32] = GATE(STS_TG32, KSEL(SEG_KCODE), irq0, DPL_KERN);
	idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_KERN);
	idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);

	set_idt(idt, sizeof(idt));

	// register event handler
	user_handler = handler;

	return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
	int* stack_ptr = (int*)stack.end;
	*(stack_ptr--) = 0x2; //eflags
	*(stack_ptr--) = 0x8; //cs
	*(stack_ptr--) = (int)entry; //eip
	*(stack_ptr--) = 0x0; //err
	*(stack_ptr--) = 0x0; //irq
	for(int i = 0; i < 8; i++){
		*(stack_ptr--) = 0x0; //general registers
	}
	*(stack_ptr)= 0x0; //prot
	*(int*)stack.start = (int)stack_ptr;
	return (_Context*)stack_ptr;
}

void _yield() {
	asm volatile("int $0x81");
}

int _intr_read() {
	return 0;
}

void _intr_write(int enable) {
}
