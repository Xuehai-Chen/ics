#ifndef __MONITOR_H__
#define __MONITOR_H__

enum { NEMU_STOP, NEMU_RUNNING, NEMU_END, NEMU_ABORT };
extern int nemu_state;
extern int is_detach_mode;

#define ENTRY_START 0x100000

#endif
