#include "common.h"

_Context* do_syscall(_Context*);

_Context *schedule(_Context*);

static _Context* do_event(_Event e, _Context* c) {
	_Context* cp;
	switch (e.event) {
		case _EVENT_IRQ_TIMER:
			Log("event: _EVENT_IRQ_TIMER");
			_yield();
			break;
		case _EVENT_SYSCALL:
			return do_syscall(c);
		case _EVENT_YIELD:
			//Log("event: _EVENT_YIELD");
			cp = schedule(c);
			//Log("prev context: %d\tscheduling context: %d", c, cp);
			return cp;
		default: panic("Unhandled event ID = %d", e.event);
	}

	return NULL;
}

void init_irq(void) {
	Log("Initializing interrupt/exception handler...");
	_cte_init(do_event);
}
