#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
	/* TODO: Trigger an interrupt/exception with ``NO''.
	 * That is, use ``NO'' to index the IDT.
	 */

	rtl_push(&cpu.eflags.val);
	rtl_push(&cpu.cs);
	rtl_push(&ret_addr);
	//GateDesc *gd = (uint64_t *)cpu.idtr.base + NO;
	t0 = vaddr_read(cpu.idtr.base + NO * 8, 4);
	rtl_andi(&t0, &t0, 0xffff);
	t1 = vaddr_read(cpu.idtr.base + NO * 8 + 4, 4);
	rtl_andi(&t1, &t1, 0xffff0000);
	rtl_add(&decoding.jmp_eip, &t0, &t1);
	rtl_j(decoding.jmp_eip);
}

void dev_raise_intr() {
}
